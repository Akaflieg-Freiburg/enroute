# iOS

## Known issues:

- [ ] Keyboard überdeckt UI (überall)
- [ ] Manche map tiles sind manchmal leer
- [ ] Beim Laden von Flugzeug aus Lib wird name nicht übernommen
- [ ] Crash beim Start wenn App lange inaktiv war (Zugriff auf Speicher)
- [ ] Map zeigt manche Linien nicht an
- [ ] Safe area funktioniert manchmal manchmal nicht (glaube wenn App im Hintergrund war)
- [ ] Höhe AMSL stimmt nicht
- [ ] Xcode warnt vor Memory leaks (verschiedene Stellen)


## Not yet implemented

- [ ] Notifications (noch nicht fertig)
- [ ] Import und Export
- [ ] Kennwörter (SSID) -> sieht schwierig aus unter iOS. Viele Changes in den letzen iOS-Versionen, von Apple nicht gern gesehen
- [ ] Automatische Verbindung mit Verkehrsdatenempfänger


# Build System

* Generate spritesheet in proper way

* Generate SNAP packages

* Update manual, onboarding

* Use Qt6.5 permissions

* Update to Qt6.4.3

* Fix problems with MacOS build

* Remove crazy large GIT LTS files from GitHub.

* Make GitHub actions work for MacOS


# Bug Fixing

* Fix problems with screenshots for google play

* Weather: Indicator for "Downloading Stations" does not work

* Android Split-View mode


# New Features

* Indicator that maps are being loaded

* METAR/TAF rewrite

* Exit app on double 'back'

* https://www.qt.io/blog/qt-for-android-storage-updates

* https://www.qt.io/blog/deploying-to-linux-with-cmake
