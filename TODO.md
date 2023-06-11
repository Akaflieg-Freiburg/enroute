# iOS

## Known issues:

- [X] Keyboard überdeckt UI (überall)
- [X] Manche map tiles sind manchmal leer
- [X] Beim Laden von Flugzeug aus Lib wird name nicht übernommen
- [ ] Crash beim Start wenn App lange inaktiv war (Zugriff auf Speicher)
- [X] Map zeigt manche Linien nicht an
- [ ] Safe area funktioniert manchmal manchmal nicht (glaube wenn App im Hintergrund war)
- [X] Höhe AMSL stimmt nicht
- [ ] Xcode warnt vor Memory leaks (verschiedene Stellen)
- [?] TextField: clear button does not work / Kebekus: removed clear buttons iOS; needs to be checked
- [?] ComboBoxes do not work when activated while the virtual keyboard is visible / Kebekus: removed ComboBoxes for iOS; needs to be checked


## Not yet implemented

- [X] Notifications (-> need to implement ourselves, Kebekus will look at that)
- [ ] Import und Export
- [X] Kennwörter (SSID) -> sieht schwierig aus unter iOS. Viele Changes in den letzen iOS-Versionen, von Apple nicht gern gesehen. Will not implement.
- [ ] Automatische Verbindung mit Verkehrsdatenempfänger


# General

- [ ] Generate spritesheet in proper way
- [ ] Update manual: onboarding, NOTAMs
- [ ] Remove crazy large GIT LTS files from GitHub.
- [ ] Sync privacy statement in enrouteText with C++ source.


## Bug Fixing

- [ ] Fix problems with upload of auto-generated screenshots for google play
- [ ] Auto-generation of screenshots conflicts with onboarding dialog


## New Features

- [ ] Indicator that maps are being loaded
- [ ] METAR/TAF rewrite
- [ ] Exit app on double 'back'
- [ ] https://www.qt.io/blog/qt-for-android-storage-updates
- [ ] https://www.qt.io/blog/deploying-to-linux-with-cmake

