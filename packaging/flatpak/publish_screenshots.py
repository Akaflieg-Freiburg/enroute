#!/usr/bin/env python3
"""
Publish the screenshots shown on Flathub.

Flathub displays the screenshots listed in the AppStream metainfo
(3rdParty/enrouteText/desktop/linux/de.akaflieg_freiburg.enroute.appdata.xml.in)
and mirrors the images when it builds the app. The images used here are the
ten-inch Google Play renders that DemoRunner writes to
fastlane/metadata/android/<locale>/images/tenInchScreenshots/<N>_<locale>.png
(a 960x600 window at a device pixel ratio of 2). Flathub's quality guidelines
ask for screenshots of decorated windows, so each render is wrapped in a
GNOME-style frame: header bar with the application title and a close button,
rounded corners, a thin outline and a soft drop shadow on a transparent
background. The screenshot pixels themselves are not altered.

The decorated files are uploaded as assets of a dedicated pre-release of the
GitHub repository and replaced in place, so the URLs in the metainfo never
change and neither the git history nor the website is touched.

Usage:
  publish_screenshots.py [--source DIR] [--appdata FILE] [--work DIR]
                         [--gh-repo OWNER/NAME] [--release TAG]
                         [--title TEXT] [--scale N] [--dry-run]

Steps:
  1. decorate every ten-inch screenshot into <work>/<locale>-<N>.png
  2. check that the metainfo references exactly these files, with the
     xml:lang attribute matching the locale of each file
  3. create the pre-release if it does not exist, then upload all files with
     `gh release upload --clobber` (skipped with --dry-run)

Requires Pillow (Fedora: python3-pillow, elsewhere: pip3 install pillow).
The upload additionally needs the GitHub CLI `gh` with a logged-in account.
"""

import argparse
import re
import shutil
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path

try:
    from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont
except ImportError:
    sys.exit('publish_screenshots: Pillow is missing; install python3-pillow '
             '(Fedora) or run `pip3 install pillow`')

REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SOURCE = REPO_ROOT / 'fastlane' / 'metadata' / 'android'
DEFAULT_APPDATA = (REPO_ROOT / '3rdParty' / 'enrouteText' / 'desktop' / 'linux'
                   / 'de.akaflieg_freiburg.enroute.appdata.xml.in')
DEFAULT_GH_REPO = 'Akaflieg-Freiburg/enroute'
DEFAULT_RELEASE = 'screenshots'
DEFAULT_TITLE = 'Enroute Flight Navigation'
DEFAULT_SCALE = 2          # device pixel ratio of DemoRunner's ten-inch renders
DEVICE_DIR = 'tenInchScreenshots'
XML_LANG = '{http://www.w3.org/XML/1998/namespace}lang'

# AppStream language tag expected on the <image> element of each locale.
# None means the untranslated (default) image.
LOCALE_LANG = {'en-US': None, 'de-DE': 'de', 'fr-FR': 'fr', 'it-IT': 'it', 'pl-PL': 'pl'}

RELEASE_TITLE = 'Flathub screenshots'
RELEASE_NOTES = ('Decorated screenshots for the Flathub listing, uploaded by the fastlane '
                 'lane flathubRelease (packaging/flatpak/publish_screenshots.py). '
                 'This is not a software release.')

# ---------------------------------------------------------------------------
# Window frame, in logical pixels (multiplied by --scale), libadwaita light look
# ---------------------------------------------------------------------------

HEADER_HEIGHT = 47
CORNER_RADIUS = 12
TITLE_SIZE = 15
CLOSE_DIAMETER = 24
CLOSE_INSET = 6            # distance between the close button and the window edge
CLOSE_CROSS = 9            # extent of the "x" inside the close button
CLOSE_STROKE = 2
MARGIN = 40                # transparent border around the window, holds the shadow
SHADOW_BLUR = 12
SHADOW_OFFSET = 6
SHADOW_OPACITY = 0.35
HEADER_BG = (255, 255, 255)
HEADER_LINE = (224, 224, 224)          # rgba(0,0,0,.12) over white
TITLE_OPACITY = 0.8
CLOSE_BG_OPACITY = 0.10
OUTLINE_OPACITY = 0.23
SUPERSAMPLE = 4            # anti-aliasing for round shapes

FONTS = (('Adwaita Sans', 'Bold'), ('Cantarell', 'Bold'))   # GNOME 48+, then older GNOME


class PublishError(Exception):
    """Anything that must stop the caller."""


# ---------------------------------------------------------------------------
# Decoration
# ---------------------------------------------------------------------------

def fc_match(family, style):
    """Font file for `family` via fontconfig, or None if unavailable or substituted."""
    if shutil.which('fc-match') is None:
        return None
    result = subprocess.run(['fc-match', '-f', '%{family}\t%{file}', f'{family}:style={style}'],
                            capture_output=True, text=True)
    if result.returncode != 0 or '\t' not in result.stdout:
        return None
    families, path = result.stdout.split('\t', 1)
    return path.strip() if family in families.split(',') else None


def load_font(size):
    """Bold UI font as used by GNOME, falling back to Pillow's bundled font."""
    for family, style in FONTS:
        path = fc_match(family, style)
        if path is None:
            continue
        font = ImageFont.truetype(path, size)
        try:
            names = [name.decode('ascii', 'ignore') for name in font.get_variation_names()]
            if style in names:
                font.set_variation_by_name(style)
        except OSError:
            pass                       # not a variable font; the file already has the style
        return font
    return ImageFont.load_default(size=size)


def round_mask(size, radius):
    """Anti-aliased 'L' mask of a rounded rectangle of `size`."""
    width, height = size
    big = Image.new('L', (width * SUPERSAMPLE, height * SUPERSAMPLE), 0)
    ImageDraw.Draw(big).rounded_rectangle(
        [0, 0, width * SUPERSAMPLE - 1, height * SUPERSAMPLE - 1],
        radius=radius * SUPERSAMPLE, fill=255)
    return big.resize(size, Image.LANCZOS)


def close_button(scale):
    """Two 'L' masks (button disc, cross glyph) of the close button, each D x D pixels."""
    diameter = CLOSE_DIAMETER * scale
    big = diameter * SUPERSAMPLE
    disc = Image.new('L', (big, big), 0)
    ImageDraw.Draw(disc).ellipse([0, 0, big - 1, big - 1], fill=255)
    cross = Image.new('L', (big, big), 0)
    draw = ImageDraw.Draw(cross)
    half = CLOSE_CROSS * scale * SUPERSAMPLE / 2
    stroke = CLOSE_STROKE * scale * SUPERSAMPLE
    centre = big / 2
    for dx, dy in ((1, 1), (1, -1)):
        start = (centre - dx * half, centre - dy * half)
        end = (centre + dx * half, centre + dy * half)
        draw.line([start, end], fill=255, width=stroke)
        for x, y in (start, end):      # round caps
            draw.ellipse([x - stroke / 2, y - stroke / 2, x + stroke / 2, y + stroke / 2], fill=255)
    return (disc.resize((diameter, diameter), Image.LANCZOS),
            cross.resize((diameter, diameter), Image.LANCZOS))


def decorate(source, target, title, scale, font):
    """Wrap the screenshot at `source` in a window frame and save it to `target`."""
    shot = Image.open(source).convert('RGB')
    width, height = shot.size
    header = HEADER_HEIGHT * scale
    window_size = (width, height + header)
    margin = MARGIN * scale

    # Window content: white header bar, separator line, screenshot below.
    window = Image.new('RGB', window_size, HEADER_BG)
    window.paste(shot, (0, header))
    draw = ImageDraw.Draw(window, 'RGBA')
    draw.rectangle([0, header - scale, width - 1, header - 1], fill=HEADER_LINE)
    draw.text((width / 2, header / 2), title, font=font, anchor='mm',
              fill=(0, 0, 0, round(255 * TITLE_OPACITY)))

    # Close button, GNOME's default layout has no minimize/maximize buttons.
    disc, cross = close_button(scale)
    diameter = disc.size[0]
    origin = (width - CLOSE_INSET * scale - diameter, (header - diameter) // 2)
    window.paste((0, 0, 0), origin, disc.point(lambda v: v * CLOSE_BG_OPACITY))
    window.paste((0, 0, 0), origin, cross.point(lambda v: v * TITLE_OPACITY))

    # Rounded corners and a one pixel outline just inside the edge.
    mask = round_mask(window_size, CORNER_RADIUS * scale)
    inner = mask.filter(ImageFilter.MinFilter(2 * scale + 1))
    outline = ImageChops.subtract(mask, inner).point(lambda v: v * OUTLINE_OPACITY)
    window.paste((0, 0, 0), (0, 0), outline)
    window = window.convert('RGBA')
    window.putalpha(mask)

    # Shadow on a transparent canvas, window on top.
    canvas_size = (window_size[0] + 2 * margin, window_size[1] + 2 * margin)
    shadow = Image.new('L', canvas_size, 0)
    shadow.paste(mask, (margin, margin + SHADOW_OFFSET * scale))
    shadow = shadow.filter(ImageFilter.GaussianBlur(SHADOW_BLUR * scale))
    canvas = Image.new('RGBA', canvas_size, (0, 0, 0, 0))
    canvas.paste((0, 0, 0, 255), (0, 0), shadow.point(lambda v: v * SHADOW_OPACITY))
    layer = Image.new('RGBA', canvas_size, (0, 0, 0, 0))
    layer.paste(window, (margin, margin))
    Image.alpha_composite(canvas, layer).save(target, optimize=True)


def find_screenshots(source):
    """{(locale, number): path} for every ten-inch screenshot below `source`."""
    found = {}
    for locale_dir in sorted(Path(source).iterdir()):
        device_dir = locale_dir / 'images' / DEVICE_DIR
        if not device_dir.is_dir():
            continue
        locale = locale_dir.name
        for path in device_dir.glob(f'*_{locale}.png'):
            match = re.fullmatch(r'(\d+)_' + re.escape(locale) + r'\.png', path.name)
            if match:
                found[(locale, int(match.group(1)))] = path
    if not found:
        raise PublishError(f'no {DEVICE_DIR} screenshots found below {source}')
    return found


def asset_name(locale, number):
    return f'{locale}-{number}.png'


def decorate_all(source, work, title, scale):
    """Decorate every screenshot; returns {asset name: (locale, path)}."""
    font = load_font(TITLE_SIZE * scale)
    work = Path(work)
    work.mkdir(parents=True, exist_ok=True)
    assets = {}
    for (locale, number), path in sorted(find_screenshots(source).items()):
        name = asset_name(locale, number)
        decorate(path, work / name, title, scale, font)
        assets[name] = (locale, work / name)
        print(f'{path.relative_to(source)} -> {work / name}')
    return assets


# ---------------------------------------------------------------------------
# Cross-check with the AppStream metainfo
# ---------------------------------------------------------------------------

def asset_url(gh_repo, release, name):
    return f'https://github.com/{gh_repo}/releases/download/{release}/{name}'


def check_appdata(appdata, assets, gh_repo, release):
    """Every <image> must be one of `assets` with the right xml:lang, and vice versa."""
    try:
        root = ET.parse(appdata).getroot()
    except (OSError, ET.ParseError) as error:
        raise PublishError(f'cannot parse {appdata}: {error}') from error
    expected = {asset_url(gh_repo, release, name): LOCALE_LANG.get(locale, locale)
                for name, (locale, _) in assets.items()}
    problems = []
    seen = set()
    for image in root.iterfind('./screenshots/screenshot/image'):
        url = (image.text or '').strip()
        lang = image.get(XML_LANG)
        if url not in expected:
            problems.append(f'{url}: not among the decorated screenshots')
        elif lang != expected[url]:
            problems.append(f'{url}: xml:lang is {lang!r}, expected {expected[url]!r}')
        elif url in seen:
            problems.append(f'{url}: referenced twice')
        seen.add(url)
    for url in sorted(set(expected) - seen):
        problems.append(f'{url}: decorated but not referenced')
    if problems:
        raise PublishError(f'{appdata} does not match the decorated screenshots:\n  '
                           + '\n  '.join(problems))
    print(f'{appdata}: references all {len(seen)} screenshots')


# ---------------------------------------------------------------------------
# Upload
# ---------------------------------------------------------------------------

def gh(*args, check=True):
    if shutil.which('gh') is None:
        raise PublishError('the GitHub CLI `gh` is not installed')
    return subprocess.run(['gh', *args], check=check)


def upload(assets, gh_repo, release):
    if gh('release', 'view', release, '--repo', gh_repo, check=False).returncode != 0:
        print(f'creating pre-release {release} in {gh_repo}')
        gh('release', 'create', release, '--repo', gh_repo, '--prerelease', '--latest=false',
           '--title', RELEASE_TITLE, '--notes', RELEASE_NOTES)
    files = [str(path) for _, path in assets.values()]
    gh('release', 'upload', release, '--repo', gh_repo, '--clobber', *files)
    for name in assets:
        print(asset_url(gh_repo, release, name))


# ---------------------------------------------------------------------------
# Command line
# ---------------------------------------------------------------------------

def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    parser.add_argument('--source', default=DEFAULT_SOURCE,
                        help=f'fastlane android metadata directory (default: {DEFAULT_SOURCE})')
    parser.add_argument('--appdata', default=DEFAULT_APPDATA,
                        help='AppStream metainfo, configured or the .in template '
                             f'(default: {DEFAULT_APPDATA})')
    parser.add_argument('--work', help='where the decorated files are written '
                                       '(default: a fresh temporary directory)')
    parser.add_argument('--gh-repo', default=DEFAULT_GH_REPO)
    parser.add_argument('--release', default=DEFAULT_RELEASE,
                        help=f'tag of the pre-release holding the assets (default: {DEFAULT_RELEASE})')
    parser.add_argument('--title', default=DEFAULT_TITLE, help='window title')
    parser.add_argument('--scale', type=int, default=DEFAULT_SCALE,
                        help=f'device pixel ratio of the renders (default: {DEFAULT_SCALE})')
    parser.add_argument('--dry-run', action='store_true', help='decorate and check, do not upload')
    args = parser.parse_args(argv)

    work = args.work or tempfile.mkdtemp(prefix='enroute-screenshots-')
    try:
        assets = decorate_all(args.source, work, args.title, args.scale)
        check_appdata(args.appdata, assets, args.gh_repo, args.release)
        if args.dry_run:
            print(f'dry run: {len(assets)} decorated screenshots in {work}, nothing uploaded')
        else:
            upload(assets, args.gh_repo, args.release)
    except PublishError as error:
        sys.exit(f'publish_screenshots: {error}')
    except subprocess.CalledProcessError as error:
        sys.exit(f'publish_screenshots: {" ".join(error.cmd)} failed with exit code {error.returncode}')


if __name__ == '__main__':
    main()
