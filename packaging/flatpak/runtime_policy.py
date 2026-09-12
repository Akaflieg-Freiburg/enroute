#!/usr/bin/env python3
"""
Choose the org.kde.Platform runtime for the enroute flatpak.

The flatpak uses Qt private API (maplibre QtLocation plugin, out-of-tree
qthttpserver module). Qt aborts at startup when the runtime's Qt patch
version differs from the one the app was compiled against, and a flatpak
runtime branch cannot be pinned: users always get the branch head. KDE bumps
the Qt patch version inside a branch for about half a year after a Qt minor
release and then leaves it alone while the branch stays supported.

Policy: build against a supported branch whose Qt series is finished
("frozen"), and hop to a newer branch only once that branch carries the
final patch release of its Qt series. A manifest is never moved to an older
branch than the one it already uses. If no frozen branch exists at or above
the manifest's current branch, the script fails and writes nothing. The
qthttpserver module is pinned to the git tag matching the runtime's Qt
version.

A branch is frozen when
  * io.qt.qtwebengine.BaseApp exists for it on flathub,
  * endoflife.date lists the Qt series as end-of-life for at least
    GRACE_EOL days (Qt stops patching a series once the next minor is out;
    the grace period covers a straggler patch release),
  * the latest patch release is at least GRACE_PATCH days old, and
  * the runtime published on flathub carries exactly that latest release.
LTS series have a far-off end-of-life date on endoflife.date and therefore
never count as frozen; KDE retires their runtime branches on the usual
schedule anyway.

Usage:
  runtime_policy.py select --manifest FILE [--reference FILE] [--today YYYY-MM-DD]
  runtime_policy.py apply  --manifest FILE [--reference FILE] [--template FILE]
                           [--today YYYY-MM-DD] [--dry-run] [--skip-tag-check]
                           [--json-out FILE]
  runtime_policy.py facts  [--output FILE]

  --manifest   manifest to inspect or rewrite (runtime-version, base-version,
               qthttpserver tag)
  --reference  another manifest whose runtime-version also counts as the
               current branch (e.g. the manifest deployed on flathub)
  --template   second file to rewrite the same way (the .json.in template)

Only the Python standard library is used and no flatpak installation is
required, so this also runs on macOS. Flathub is read as the plain OSTree
repository it is: refs/heads/<ref> names the published commit, the commit
object carries the end-of-life marker, and the tree objects list the files
of the runtime, from which the Qt version is taken (libQt6Core soname). A few
kilobytes are downloaded per branch.
"""

import argparse
import difflib
import json
import re
import subprocess
import sys
import urllib.error
import urllib.request
from dataclasses import dataclass, field
from datetime import date, timedelta

RUNTIME = 'org.kde.Platform'
BASEAPP = 'io.qt.qtwebengine.BaseApp'
ARCH = 'x86_64'
FLATHUB_REPO = 'https://dl.flathub.org/repo'
EOL_URL = 'https://endoflife.date/api/qt.json'
QTHTTPSERVER_URL = 'https://github.com/qt/qthttpserver.git'
GRACE_EOL = timedelta(days=45)
GRACE_PATCH = timedelta(days=30)
BRANCH_RE = re.compile(r'^6\.\d+$')
SONAME_RE = re.compile(r'^libQt6Core\.so\.(6\.\d+\.\d+)$')


class PolicyError(Exception):
    """Anything that must stop the caller without touching any file."""


def branch_key(branch):
    return tuple(int(part) for part in branch.split('.'))


# ---------------------------------------------------------------------------
# Data
# ---------------------------------------------------------------------------

@dataclass
class Cycle:
    """Lifecycle data of one Qt minor series, as published by endoflife.date."""
    latest: str
    latest_release_date: date
    eol: date
    lts: bool = False


@dataclass
class Facts:
    """Everything the policy needs to know about flathub and Qt."""
    supported: list             # org.kde.Platform 6.x branches on flathub, EOL ones excluded
    baseapp: list               # branches for which io.qt.qtwebengine.BaseApp exists
    cycles: dict                # "6.10" -> Cycle
    runtime_qt: dict = field(default_factory=dict)   # branch -> Qt version in the published runtime
    fetch_runtime_qt: object = None                  # callable(branch) -> Qt version, or None

    def qt_of(self, branch):
        """Qt version published in the runtime of `branch`, fetched on first use."""
        if branch not in self.runtime_qt:
            if self.fetch_runtime_qt is None:
                raise PolicyError(f'Qt version of {RUNTIME}//{branch} is unknown')
            self.runtime_qt[branch] = self.fetch_runtime_qt(branch)
        return self.runtime_qt[branch]

    def to_json(self):
        return {
            'supported': self.supported,
            'baseapp': self.baseapp,
            'runtime_qt': self.runtime_qt,
            'cycles': {
                name: {
                    'latest': cycle.latest,
                    'latestReleaseDate': cycle.latest_release_date.isoformat(),
                    'eol': cycle.eol.isoformat(),
                    'lts': cycle.lts,
                }
                for name, cycle in self.cycles.items()
            },
        }


@dataclass
class Decision:
    branch: str
    qt: str
    reason: str


# ---------------------------------------------------------------------------
# Pure parsing and policy (unit-tested, no I/O)
# ---------------------------------------------------------------------------

def parse_cycles(entries):
    """Turn the endoflife.date product list into Cycle objects keyed by minor version."""
    cycles = {}
    for entry in entries:
        name = str(entry.get('cycle', ''))
        latest = entry.get('latest')
        released = entry.get('latestReleaseDate')
        eol = entry.get('eol')
        if not BRANCH_RE.match(name) or not latest or not released:
            continue
        if eol is True:
            eol_date = date(1970, 1, 1)
        elif eol is False or eol is None:
            eol_date = date.max
        else:
            eol_date = date.fromisoformat(str(eol))
        cycles[name] = Cycle(latest=str(latest),
                             latest_release_date=date.fromisoformat(str(released)),
                             eol=eol_date,
                             lts=bool(entry.get('lts', False)))
    return cycles


def qt_version_from_names(names):
    """Qt version encoded in the libQt6Core soname among `names`."""
    versions = [match.group(1) for match in map(SONAME_RE.match, names) if match]
    if not versions:
        raise PolicyError('no libQt6Core.so.6.x.y found')
    return max(versions, key=branch_key)


def freeze_blocker(branch, facts, today):
    """Why `branch` is not frozen, or None if its Qt series is finished and shipped."""
    cycle = facts.cycles.get(branch)
    if branch not in facts.baseapp:
        return f'no {BASEAPP} for {branch}'
    if cycle is None:
        return f'endoflife.date knows no Qt {branch} series'
    if today < cycle.eol + GRACE_EOL:
        return f'Qt {branch} series may still receive patches until {cycle.eol + GRACE_EOL}'
    if today < cycle.latest_release_date + GRACE_PATCH:
        return f'Qt {cycle.latest} is younger than {GRACE_PATCH.days} days'
    qt = facts.qt_of(branch)
    if qt != cycle.latest:
        return f'runtime ships Qt {qt}, final release is {cycle.latest}'
    return None


def is_frozen(branch, facts, today):
    return freeze_blocker(branch, facts, today) is None


def decide(current, facts, today):
    """Pick the runtime branch for a manifest currently on `current` (a branch or None).

    Raises PolicyError when no frozen branch at or above `current` exists.
    """
    candidates = [b for b in facts.supported
                  if current is None or branch_key(b) >= branch_key(current)]
    if not candidates:
        raise PolicyError(f'flathub offers no supported {RUNTIME} branch at or above {current}')
    blockers = {b: freeze_blocker(b, facts, today) for b in candidates}
    frozen = [b for b in candidates if blockers[b] is None]
    if not frozen:
        details = '; '.join(f'{b}: {blockers[b]}' for b in candidates)
        raise PolicyError(f'no frozen {RUNTIME} branch available ({details})')
    target = frozen[-1]
    if target == current:
        reason = f'{target} is frozen'
    elif current in facts.supported:
        reason = f'{target} is the newest frozen branch, hopping from {current}'
    else:
        reason = f'{target} is the newest frozen branch'
    return Decision(branch=target, qt=facts.qt_of(target), reason=reason)


# ---------------------------------------------------------------------------
# Manifest handling
# ---------------------------------------------------------------------------

def render(doc):
    return json.dumps(doc, indent=2) + '\n'


def load_manifest(path, strict=True):
    with open(path, encoding='utf-8') as handle:
        text = handle.read()
    doc = json.loads(text)
    if strict and render(doc) != text:
        raise PolicyError(f'{path} would not survive a rewrite unchanged, refusing to touch it')
    return doc


def qthttpserver_source(doc):
    for module in doc.get('modules', []):
        if isinstance(module, dict) and module.get('name') == 'qthttpserver':
            return module['sources'][0]
    raise PolicyError('manifest has no module named qthttpserver')


def manifest_fields(doc):
    return doc.get('runtime-version'), doc.get('base-version'), qthttpserver_source(doc).get('tag')


def set_fields(doc, branch, qt):
    doc['runtime-version'] = branch
    if 'base-version' in doc:
        doc['base-version'] = branch
    qthttpserver_source(doc)['tag'] = 'v' + qt


def current_branch(doc):
    value = doc.get('runtime-version')
    return value if isinstance(value, str) and BRANCH_RE.match(value) else None


def rewrite(path, decision, dry_run):
    """Apply `decision` to the manifest at `path`; returns True if the file differs."""
    doc = load_manifest(path)
    old = render(doc)
    file_branch = current_branch(doc)
    if file_branch and branch_key(file_branch) > branch_key(decision.branch):
        print(f'{path}: already on {file_branch}, newer than {decision.branch}; left unchanged')
        return False
    set_fields(doc, decision.branch, decision.qt)
    new = render(doc)
    if new == old:
        print(f'{path}: already up to date')
        return False
    sys.stdout.writelines(difflib.unified_diff(
        old.splitlines(keepends=True), new.splitlines(keepends=True),
        fromfile=path, tofile=path))
    if dry_run:
        print(f'{path}: not written (dry run)')
    else:
        with open(path, 'w', encoding='utf-8') as handle:
            handle.write(new)
        print(f'{path}: updated')
    return True


# ---------------------------------------------------------------------------
# GVariant deserialisation (the subset OSTree objects need)
# ---------------------------------------------------------------------------

_FIXED = {'y': 1, 'b': 1, 'n': 2, 'q': 2, 'i': 4, 'u': 4, 'x': 8, 't': 8, 'd': 8}


class _Type:
    __slots__ = ('code', 'children', 'fixed', 'align')

    def __init__(self, code, children=(), fixed=None, align=1):
        self.code, self.children, self.fixed, self.align = code, children, fixed, align


def _align_up(value, alignment):
    return (value + alignment - 1) // alignment * alignment


def _parse_type(signature, pos=0):
    code = signature[pos]
    if code in _FIXED:
        return _Type(code, fixed=_FIXED[code], align=_FIXED[code]), pos + 1
    if code in 'sog':
        return _Type(code), pos + 1
    if code == 'v':
        return _Type('v', align=8), pos + 1
    if code == 'a':
        child, pos = _parse_type(signature, pos + 1)
        return _Type('a', (child,), align=child.align), pos
    if code in '({':
        closing = ')' if code == '(' else '}'
        pos += 1
        children = []
        while signature[pos] != closing:
            child, pos = _parse_type(signature, pos)
            children.append(child)
        result = _Type(code, tuple(children), align=max([child.align for child in children] or [1]))
        if all(child.fixed for child in children):
            size = 0
            for child in children:
                size = _align_up(size, child.align) + child.fixed
            result.fixed = _align_up(size, result.align) or 1
        return result, pos + 1
    raise ValueError(f'unsupported GVariant type {code!r} in {signature!r}')


def _offset_size(size):
    return 1 if size <= 0xff else 2 if size <= 0xffff else 4 if size <= 0xffffffff else 8


def _read_le(data):
    return int.from_bytes(data, 'little')


def _deserialize(gtype, data):
    """Decode `data` (a memoryview) of GVariant type `gtype`; integers are big-endian as in OSTree."""
    code = gtype.code
    if code in _FIXED:
        return int.from_bytes(data, 'big')
    if code in 'sog':
        return bytes(data[:-1]).decode('utf-8') if len(data) else ''
    if code == 'v':
        split = bytes(data).rindex(b'\x00')
        inner, _ = _parse_type(bytes(data[split + 1:]).decode('ascii'))
        return _deserialize(inner, data[:split])
    if code == 'a':
        child = gtype.children[0]
        if child.code == 'y':
            return bytes(data)
        if not len(data):
            return []
        if child.fixed:
            return [_deserialize(child, data[i:i + child.fixed]) for i in range(0, len(data), child.fixed)]
        osz = _offset_size(len(data))
        count = (len(data) - _read_le(data[-osz:])) // osz
        table = len(data) - count * osz
        items, start = [], 0
        for i in range(count):
            end = _read_le(data[table + i * osz:table + (i + 1) * osz])
            items.append(_deserialize(child, data[start:end]))
            start = _align_up(end, child.align)
        return items
    if code in '({':
        members = gtype.children
        last = len(members) - 1
        osz = _offset_size(len(data))
        offsets = sum(1 for i, member in enumerate(members) if not member.fixed and i != last)
        values, pos, used = [], 0, 0
        for i, member in enumerate(members):
            pos = _align_up(pos, member.align)
            if member.fixed:
                end = pos + member.fixed
            elif i != last:
                end = _read_le(data[len(data) - osz * (used + 1):len(data) - osz * used])
                used += 1
            else:
                end = len(data) - offsets * osz
            values.append(_deserialize(member, data[pos:end]))
            pos = end
        return tuple(values)
    raise ValueError(f'unsupported GVariant type {code!r}')


def gvariant(signature, data):
    gtype, _ = _parse_type(signature)
    return _deserialize(gtype, memoryview(data))


def parse_commit(data):
    """OSTree commit object -> (metadata dict, subject, root dirtree checksum)."""
    metadata, _parent, _related, subject, _body, _timestamp, root_tree, _root_meta = gvariant(
        '(a{sv}aya(say)sstayay)', data)
    return dict(metadata), subject, root_tree.hex()


def parse_dirtree(data):
    """OSTree dirtree object -> ({file name: checksum}, {dir name: dirtree checksum})."""
    files, dirs = gvariant('(a(say)a(sayay))', data)
    return {name: checksum.hex() for name, checksum in files}, {name: tree.hex() for name, tree, _meta in dirs}


# ---------------------------------------------------------------------------
# I/O: flathub's OSTree repository, endoflife.date, git
# ---------------------------------------------------------------------------

def http_get(url, missing_ok=False):
    request = urllib.request.Request(url, headers={'User-Agent': 'enroute-runtime-policy'})
    try:
        with urllib.request.urlopen(request, timeout=60) as response:
            return response.read()
    except urllib.error.HTTPError as error:
        if missing_ok and error.code == 404:
            return None
        raise PolicyError(f'cannot fetch {url}: {error}') from error
    except Exception as error:  # noqa: BLE001 - any failure must stop the caller
        raise PolicyError(f'cannot fetch {url}: {error}') from error


class Flathub:
    """Read-only view of flathub's OSTree repository over HTTPS."""

    def __init__(self, repo=FLATHUB_REPO, arch=ARCH):
        self.repo, self.arch = repo, arch
        self._heads = {}

    def head(self, ref):
        """Checksum of the published commit for `ref`, or None if the ref does not exist."""
        if ref not in self._heads:
            data = http_get(f'{self.repo}/refs/heads/{ref}', missing_ok=True)
            self._heads[ref] = data.decode('ascii').strip() if data else None
        return self._heads[ref]

    def object(self, checksum, kind):
        return http_get(f'{self.repo}/objects/{checksum[:2]}/{checksum[2:]}.{kind}')

    def runtime_ref(self, branch):
        return f'runtime/{RUNTIME}/{self.arch}/{branch}'

    def baseapp_ref(self, branch):
        return f'app/{BASEAPP}/{self.arch}/{branch}'

    def runtime_state(self, branch):
        """'supported', 'eol', or 'missing' for the runtime branch."""
        checksum = self.head(self.runtime_ref(branch))
        if checksum is None:
            return 'missing'
        metadata, _subject, _root = parse_commit(self.object(checksum, 'commit'))
        return 'eol' if 'ostree.endoflife' in metadata or 'ostree.endoflife-rebase' in metadata else 'supported'

    def runtime_qt_version(self, branch):
        """Qt version inside the published runtime, from the libQt6Core soname."""
        checksum = self.head(self.runtime_ref(branch))
        if checksum is None:
            raise PolicyError(f'{self.runtime_ref(branch)} does not exist on flathub')
        _metadata, _subject, tree = parse_commit(self.object(checksum, 'commit'))
        for name in ('files', 'lib', f'{self.arch}-linux-gnu'):
            _files, dirs = parse_dirtree(self.object(tree, 'dirtree'))
            if name not in dirs:
                raise PolicyError(f'{self.runtime_ref(branch)}: directory {name} not found')
            tree = dirs[name]
        files, _dirs = parse_dirtree(self.object(tree, 'dirtree'))
        return qt_version_from_names(files)


def fetch_cycles():
    entries = json.loads(http_get(EOL_URL))
    cycles = parse_cycles(entries)
    if not cycles:
        raise PolicyError(f'{EOL_URL} returned no usable Qt 6 cycles')
    return cycles


def tag_exists(qt):
    ref = f'refs/tags/v{qt}'
    result = subprocess.run(['git', 'ls-remote', '--tags', QTHTTPSERVER_URL, ref],
                            capture_output=True, text=True)
    if result.returncode != 0:
        raise PolicyError(f'git ls-remote {QTHTTPSERVER_URL} failed: {result.stderr.strip()}')
    return any(line.split('\t')[-1] == ref for line in result.stdout.splitlines())


def gather_facts(current=None, flathub=None):
    """Facts for every Qt 6 series known to endoflife.date, at or above `current` if given."""
    flathub = flathub or Flathub()
    cycles = fetch_cycles()
    branches = sorted((name for name in cycles
                       if current is None or branch_key(name) >= branch_key(current)), key=branch_key)
    supported = [b for b in branches if flathub.runtime_state(b) == 'supported']
    baseapp = [b for b in branches if flathub.head(flathub.baseapp_ref(b)) is not None]
    if not supported:
        raise PolicyError(f'flathub lists no supported {RUNTIME} 6.x runtime'
                          + (f' at or above {current}' if current else ''))
    return Facts(supported=supported, baseapp=baseapp, cycles=cycles,
                 fetch_runtime_qt=flathub.runtime_qt_version)


# ---------------------------------------------------------------------------
# Command line
# ---------------------------------------------------------------------------

def resolve_current(args):
    """Highest runtime branch named by the manifest and the optional reference manifest."""
    branches = [current_branch(load_manifest(args.manifest))]
    if args.reference:
        branches.append(current_branch(load_manifest(args.reference, strict=False)))
    branches = [branch for branch in branches if branch]
    return max(branches, key=branch_key) if branches else None


def run_policy(args):
    today = date.fromisoformat(args.today) if args.today else date.today()
    current = resolve_current(args)
    facts = gather_facts(current)
    decision = decide(current, facts, today)
    print(f'Supported runtimes: {", ".join(facts.supported)}; base app for: {", ".join(facts.baseapp)}')
    print(f'Current branch: {current or "none"}')
    print(f'Decision: {RUNTIME}//{decision.branch} with Qt {decision.qt}: {decision.reason}')
    return current, facts, decision


def cmd_select(args):
    _, _, decision = run_policy(args)
    rewrite(args.manifest, decision, dry_run=True)


def cmd_apply(args):
    current, _, decision = run_policy(args)
    if not args.skip_tag_check and not tag_exists(decision.qt):
        raise PolicyError(f'{QTHTTPSERVER_URL} has no tag v{decision.qt}; cannot pin qthttpserver')
    changed = [path for path in [args.manifest, args.template]
               if path and rewrite(path, decision, args.dry_run)]
    if args.json_out:
        with open(args.json_out, 'w', encoding='utf-8') as handle:
            json.dump({'branch': decision.branch, 'qt': decision.qt,
                       'reason': decision.reason, 'previous': current, 'changed': changed},
                      handle, indent=2)
            handle.write('\n')


def cmd_facts(args):
    facts = gather_facts()
    for branch in facts.supported:
        facts.qt_of(branch)
    text = json.dumps(facts.to_json(), indent=2) + '\n'
    if args.output:
        with open(args.output, 'w', encoding='utf-8') as handle:
            handle.write(text)
    else:
        sys.stdout.write(text)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    commands = parser.add_subparsers(dest='command', required=True)

    def add_policy_args(sub):
        sub.add_argument('--manifest', required=True)
        sub.add_argument('--reference')
        sub.add_argument('--today', help='override the current date (YYYY-MM-DD)')

    select = commands.add_parser('select', help='print the decision and the diff it implies')
    add_policy_args(select)
    select.set_defaults(func=cmd_select)

    apply_ = commands.add_parser('apply', help='rewrite the manifest (and template) accordingly')
    add_policy_args(apply_)
    apply_.add_argument('--template')
    apply_.add_argument('--dry-run', action='store_true')
    apply_.add_argument('--skip-tag-check', action='store_true')
    apply_.add_argument('--json-out')
    apply_.set_defaults(func=cmd_apply)

    facts = commands.add_parser('facts', help='dump what flathub and endoflife.date report')
    facts.add_argument('--output')
    facts.set_defaults(func=cmd_facts)

    args = parser.parse_args(argv)
    try:
        args.func(args)
    except PolicyError as error:
        sys.exit(f'runtime_policy: {error}')


if __name__ == '__main__':
    main()
