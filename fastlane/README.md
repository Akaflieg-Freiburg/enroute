fastlane documentation
----

# Installation

Make sure you have the latest version of the Xcode command line tools installed:

```sh
xcode-select --install
```

For _fastlane_ installation instructions, see [Installing _fastlane_](https://docs.fastlane.tools/#installing-fastlane)

# Available Actions

## Android

### android build

```sh
[bundle exec] fastlane android build
```

Build Android Binary

### android metadata

```sh
[bundle exec] fastlane android metadata
```

Updata Meta Data for Google Play

### android validate

```sh
[bundle exec] fastlane android validate
```

Validate a new version with Google Play

### android deployBeta

```sh
[bundle exec] fastlane android deployBeta
```

Deploy a new version to Google Play - Beta

### android promoteBetaToRelease

```sh
[bundle exec] fastlane android promoteBetaToRelease
```

Promote Beta to Release

----


## linux

### linux build

```sh
[bundle exec] fastlane linux build
```

Build Linux Binary

### linux flathub

```sh
[bundle exec] fastlane linux flathub
```

Build on Flathub

----


## Mac

### mac build

```sh
[bundle exec] fastlane mac build
```

Build macOS Bundle

----

This README.md is auto-generated and will be re-generated every time [_fastlane_](https://fastlane.tools) is run.

More information about _fastlane_ can be found on [fastlane.tools](https://fastlane.tools).

The documentation of _fastlane_ can be found on [docs.fastlane.tools](https://docs.fastlane.tools).
