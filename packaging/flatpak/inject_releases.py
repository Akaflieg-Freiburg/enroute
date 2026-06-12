#!/usr/bin/env python3
"""Splice release notes from CHANGELOG.md into an AppStream metainfo file.

Converts the Markdown changelog into an AppStream <releases> element and
replaces the stub <releases> element of the metainfo file, so that release
notes are presented to users on Flathub and in software centers.

Usage: inject_releases.py <CHANGELOG.md> <metainfo.xml>
"""

import re
import sys
from xml.sax.saxutils import escape


def sanitize(line):
    # AppStream forbids plaintext URLs in release descriptions
    line = re.sub(r'\(?\bhttps?://[^\s)]+\)?', '', line)
    return re.sub(r'\s{2,}', ' ', line).strip()


def releases_from_changelog(text):
    pattern = re.compile(
        r'^##\s*\[?([0-9][0-9.]*)\]?\s*-\s*(\d{4})-(\d{1,2})-(\d{1,2})\s*\n(.*?)(?=^##\s|\Z)',
        re.M | re.S)
    releases = []
    for version, year, month, day, body in pattern.findall(text):
        date = f'{year}-{int(month):02d}-{int(day):02d}'
        parts = []
        items = []

        def flush_items():
            if items:
                parts.append('        <ul>\n'
                             + ''.join(f'          <li>{i}</li>\n' for i in items)
                             + '        </ul>')
                items.clear()

        for line in body.splitlines():
            line = line.strip()
            if not line:
                continue
            if line.startswith('### '):
                flush_items()
                parts.append(f'        <p>{escape(line[4:].strip())}</p>')
            elif line.startswith('- '):
                items.append(escape(sanitize(line[2:])))
            elif items:
                # Continuation of the previous list item
                if sanitize(line):
                    items[-1] += ' ' + escape(sanitize(line))
            elif sanitize(line):
                parts.append(f'        <p>{escape(sanitize(line))}</p>')
        flush_items()

        release = f'    <release version="{version}" date="{date}">'
        if parts:
            release += ('\n      <description>\n'
                        + '\n'.join(parts)
                        + '\n      </description>\n    ')
        release += '</release>'
        releases.append(release)

    if not releases:
        sys.exit('No releases found in changelog')
    return '<releases>\n' + '\n'.join(releases) + '\n  </releases>'


def main():
    if len(sys.argv) != 3:
        sys.exit(f'Usage: {sys.argv[0]} <CHANGELOG.md> <metainfo.xml>')
    changelog_path, metainfo_path = sys.argv[1], sys.argv[2]

    with open(changelog_path, encoding='utf-8') as f:
        releases = releases_from_changelog(f.read())
    with open(metainfo_path, encoding='utf-8') as f:
        metainfo = f.read()

    new_metainfo, count = re.subn(r'<releases>.*</releases>',
                                  lambda m: releases, metainfo, flags=re.S)
    if count != 1:
        sys.exit(f'Expected exactly one <releases> element in {metainfo_path}, '
                 f'found {count}')

    with open(metainfo_path, 'w', encoding='utf-8') as f:
        f.write(new_metainfo)
    print(f'Spliced {releases.count("<release ")} releases into {metainfo_path}')


if __name__ == '__main__':
    main()
