/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
 *   stefan.kebekus@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QSettings>
#include <QVariant>
#include <QtMath>

#include "AviationUnits.h"
#include "Navigation_SatNav.h"


// Static instance of this class
Q_GLOBAL_STATIC(Navigation::SatNav, satNavStatic);


Navigation::SatNav::SatNav(QObject *parent)
    : QObject(parent)
{
    source = QGeoPositionInfoSource::createDefaultSource(this);

    if (source != nullptr) {
        sourceStatus = source->error();
        connect(source, SIGNAL(error(QGeoPositionInfoSource::Error)), this, SLOT(error(QGeoPositionInfoSource::Error)));
        connect(source, &QGeoPositionInfoSource::updateTimeout, this, &SatNav::timeout);
        connect(source, &QGeoPositionInfoSource::positionUpdated, this, &SatNav::statusUpdate);
    }

    QSettings settings;
    QGeoCoordinate tmp;
    tmp.setLatitude(settings.value(QStringLiteral("SatNav/lastValidLatitude"), _lastValidCoordinate.latitude()).toDouble());
    tmp.setLongitude(settings.value(QStringLiteral("SatNav/lastValidLongitude"), _lastValidCoordinate.longitude()).toDouble());
    tmp.setAltitude(settings.value(QStringLiteral("SatNav/lastValidAltitude"), _lastValidCoordinate.altitude()).toDouble());
    altitudeCorrectionInM = settings.value(QStringLiteral("SatNav/altitudeCorrection"), 0).toInt();
    if ((tmp.type() == QGeoCoordinate::Coordinate2D) || (tmp.type() == QGeoCoordinate::Coordinate3D)) {
        _lastValidCoordinate = tmp;
    }
    _lastValidTrack = qBound(0, settings.value(QStringLiteral("SatNav/lastValidTrack"), 0).toInt(), 359);

    if (source != nullptr) {
        source->startUpdates();
        if ((source->supportedPositioningMethods() & QGeoPositionInfoSource::SatellitePositioningMethods) == QGeoPositionInfoSource::SatellitePositioningMethods) {
            _geoid = new Geoid;
        }
    }

    // Adjust and connect timeoutCounter
    timeoutCounter.setSingleShot(true);
    connect(&timeoutCounter, &QTimer::timeout, this, &SatNav::timeout);
}


Navigation::SatNav::~SatNav()
{
    QSettings settings;

    settings.setValue(QStringLiteral("SatNav/lastValidLatitude"), _lastValidCoordinate.latitude());
    settings.setValue(QStringLiteral("SatNav/lastValidLongitude"), _lastValidCoordinate.longitude());
    settings.setValue(QStringLiteral("SatNav/lastValidAltitude"), _lastValidCoordinate.altitude());
    settings.setValue(QStringLiteral("SatNav/lastValidTrack"), _lastValidTrack);
    settings.setValue(QStringLiteral("SatNav/altitudeCorrection"), altitudeCorrectionInM);
    delete source;
    delete _geoid;
}


auto Navigation::SatNav::altitudeInFeet() const -> int
{
    if (lastInfo.coordinate().type() != QGeoCoordinate::Coordinate3D) {
        return 0;
    }

    auto correction = AviationUnits::Distance::fromM(altitudeCorrectionInM);
    return qRound(rawAltitudeInFeet() + correction.toFeet());
}


auto Navigation::SatNav::altitudeInFeetAsString() const -> QString
{
    if (lastInfo.coordinate().type() != QGeoCoordinate::Coordinate3D) {
        return QStringLiteral("-");
    }

    return myLocale.toString(altitudeInFeet()) + " ft";
}


void Navigation::SatNav::setAltitudeInFeet(int altitudeInFeet)
{
    if (lastInfo.coordinate().type() != QGeoCoordinate::Coordinate3D) {
        return;
    }

    auto altCorrection = AviationUnits::Distance::fromFT(altitudeInFeet-rawAltitudeInFeet());

    altitudeCorrectionInM = qRound(altCorrection.toM());
    QSettings settings;
    settings.setValue(QStringLiteral("SatNav/altitudeCorrection"), altitudeCorrectionInM);
    emit update();
}


void Navigation::SatNav::error(QGeoPositionInfoSource::Error newSourceStatus)
{
    // Save old status and set sourceStatus to QGeoPositionInfoSource::NoError
    auto oldStatus = status();
    sourceStatus = newSourceStatus;

    // If there really is an error, reset lastInfo and cancel all counters
    if (newSourceStatus != QGeoPositionInfoSource::NoError) {
        lastInfo = QGeoPositionInfo();
        timeoutCounter.stop();
    }

    if (oldStatus != status()) {
        emit iconChanged();
        emit statusChanged();
        emit update();
    }
}


auto Navigation::SatNav::rawAltitudeInFeet() const -> int
{
    if (lastInfo.coordinate().type() != QGeoCoordinate::Coordinate3D) {
        return 0;
    }

    auto alt = AviationUnits::Distance::fromM(lastInfo.coordinate().altitude());
    return qRound(alt.toFeet() - geoidalSeparation());
}


auto Navigation::SatNav::rawAltitudeInFeetAsString() const -> QString
{
    if (lastInfo.coordinate().type() != QGeoCoordinate::Coordinate3D) {
        return QStringLiteral("-");
    }

    return myLocale.toString(rawAltitudeInFeet()) + " ft";
}


auto Navigation::SatNav::geoidalSeparation() const -> int
{
    auto corr = AviationUnits::Distance::fromM(_lastValidGeoidCorrection);
    return qRound(corr.toFeet());
}


auto Navigation::SatNav::geoidalSeparationAsString() const -> QString
{
    if (_geoid == nullptr || !_geoid->valid() || !lastInfo.isValid()) {
        return QStringLiteral("-");
    }

    return myLocale.toString(geoidalSeparation()) + " ft";
}


auto Navigation::SatNav::globalInstance() -> SatNav *
{
    return satNavStatic;
}


auto Navigation::SatNav::groundSpeedInKnots() const -> int
{
    auto gsInMPS = groundSpeedInMetersPerSecond();

    if (gsInMPS < 0.0) {
        return -1;
    }

    auto groundSpeed = AviationUnits::Speed::fromMPS(gsInMPS);
    return qRound(groundSpeed.toKT());
}


auto Navigation::SatNav::groundSpeedInKnotsAsString() const -> QString
{
    auto gsInKnots = groundSpeedInKnots();

    if (gsInKnots < 0) {
        return QStringLiteral("-");
    }
    return myLocale.toString(gsInKnots) + " kt";
}


auto Navigation::SatNav::groundSpeedInKMHAsString() const -> QString
{
    auto gsInMPS = groundSpeedInMetersPerSecond();

    if (gsInMPS < 0.0) {
        return QStringLiteral("-");
    }

    auto gsInKMH = qRound(AviationUnits::Speed::fromMPS(gsInMPS).toKMH());
    return myLocale.toString(gsInKMH) + " km/h";
}


auto Navigation::SatNav::groundSpeedInMetersPerSecond() const -> qreal
{
    if (!lastInfo.isValid()) {
        return -1.0;
    }
    if (!lastInfo.hasAttribute(QGeoPositionInfo::GroundSpeed)) {
        return -1.0;
    }

    auto groundSpeed = lastInfo.attribute(QGeoPositionInfo::GroundSpeed);
    if (!qIsFinite(groundSpeed)) {
        return -1.0;
    }
    if (groundSpeed < 0.0) {
        return -1.0;
    }

    return groundSpeed;
}


auto Navigation::SatNav::horizontalPrecisionInMeters() const -> int
{
    if (!lastInfo.isValid()) {
        return -1;
    }
    if (!lastInfo.hasAttribute(QGeoPositionInfo::HorizontalAccuracy)) {
        return -1;
    }

    auto horizontalPrecision = lastInfo.attribute(QGeoPositionInfo::HorizontalAccuracy);
    if (!qIsFinite(horizontalPrecision)) {
        return -1;
    }
    if (horizontalPrecision <= 0.0) {
        return -1;
    }

    return static_cast<int>(qFloor(horizontalPrecision+0.5));
}


auto Navigation::SatNav::horizontalPrecisionInMetersAsString() const -> QString
{
    auto _horizontalPrecisionInMeters = horizontalPrecisionInMeters();

    if (_horizontalPrecisionInMeters < 0) {
        return QStringLiteral("-");
    }
    return QStringLiteral("±%1 m").arg(_horizontalPrecisionInMeters);
}


auto Navigation::SatNav::icon() const -> QString
{
    if (status() != OK) {
        return QStringLiteral("/icons/self-noSatNav.svg");
    }

    if (track() < 0) {
        return QStringLiteral("/icons/self-noDirection.svg");
    }

    return QStringLiteral("/icons/self-withDirection.svg");
}


auto Navigation::SatNav::lastValidCoordinate() const -> QGeoCoordinate
{
    return _lastValidCoordinate;
}


auto Navigation::SatNav::lastValidCoordinateStatic() -> QGeoCoordinate
{
    // Find out that unit system we should use
    auto *_satNav = SatNav::globalInstance();
    if (_satNav != nullptr) {
        return _satNav->lastValidCoordinate();
    }
    // Fallback in the very unlikely case that no global object exists
    return QGeoCoordinate();
}


auto Navigation::SatNav::latitudeAsString() const -> QString
{
    if (!lastInfo.isValid()) {
        return QStringLiteral("-");
    }

    auto x = lastInfo.coordinate().latitude();
    if ((!qIsFinite(x)) || (x < -90.0) || (x > 90.0)) {
        return QStringLiteral("-");
    }

    auto angle = AviationUnits::Angle::fromDEG(x);
    QString latString = angle.toString();
    latString += (x >= 0) ? QStringLiteral(" N") : QStringLiteral(" S");
    return latString;
}


auto Navigation::SatNav::longitudeAsString() const -> QString
{
    if (!lastInfo.isValid()) {
        return QStringLiteral("-");
    }

    auto x = lastInfo.coordinate().longitude();
    if ((!qIsFinite(x)) || (x < -180.0) || (x > 180.0)) {
        return QStringLiteral("-");
    }

    auto angle = AviationUnits::Angle::fromDEG(x);
    QString lonString = angle.toString();

    lonString += (x >= 0) ? QStringLiteral(" E") : QStringLiteral(" W");
    return lonString;
}


auto Navigation::SatNav::status() const -> SatNav::Status
{
    if (source == nullptr) {
        return Status::Error;
    }

    if (sourceStatus != QGeoPositionInfoSource::NoError) {
        return Status::Error;
    }

    if (!timeoutCounter.isActive()) {
        return Status::Timeout;
    }

    return Status::OK;
}


auto Navigation::SatNav::statusAsString() const -> QString
{
    if (source == nullptr) {
        return tr("Not installed or access denied");
    }

    if (sourceStatus == QGeoPositionInfoSource::AccessError) {
        return tr("Access denied");
    }

    if (sourceStatus == QGeoPositionInfoSource::ClosedError) {
        return tr("Connection to satellite system lost");
    }

    if (sourceStatus != QGeoPositionInfoSource::NoError) {
        return tr("Unknown error");
    }

    if (!timeoutCounter.isActive()) {
        return tr("Waiting for signal");
    }

    return tr("OK");
}


void Navigation::SatNav::statusUpdate(const QGeoPositionInfo &info)
{
    // Ignore invalid infos
    if (!info.isValid()) {
        return;
    }

    // Save old values and set sourceStatus to QGeoPositionInfoSource::NoError
    auto oldStatus = status();
    auto oldIcon   = icon();
    sourceStatus = QGeoPositionInfoSource::NoError;

    lastInfo = info;
    timeoutCounter.start(timeoutThreshold);

    // Inform others
    if (oldStatus != Status::OK) {
        emit statusChanged();
    }

    _lastValidCoordinate = info.coordinate();
    if (_geoid != nullptr) {
        _lastValidGeoidCorrection = _geoid->operator()(static_cast<qreal>(lastInfo.coordinate().latitude()), static_cast<qreal>(lastInfo.coordinate().longitude()));
    }

    emit update();

    // Change _isInFlight if appropriate.
    if (_isInFlight) {
        // If we are in flight at present, go back to ground mode only if the ground speed is less than minFlightSpeedInKT-flightSpeedHysteresis
        if (groundSpeedInKnots() < minFlightSpeedInKT-flightSpeedHysteresis) {
            _isInFlight = false;
            emit isInFlightChanged();
        }
    } else {
        // If we are on the ground at present, go to flight mode only if the ground sped is more than minFlightSpeedInKT
        if (groundSpeedInKnots() > minFlightSpeedInKT) {
            _isInFlight = true;
            emit isInFlightChanged();
        }
    }

    // emit iconChanged() if appropriate
    if (oldIcon != icon()) {
        emit iconChanged();
    }

    // emit lastValidTrackChanged if appropriate
    auto newTrack = track();
    if ((newTrack >= 0) && (newTrack != _lastValidTrack)) {
        _lastValidTrack = newTrack;
        emit lastValidTrackChanged();
    }
}


void Navigation::SatNav::timeout()
{
    // Clear lastInfo, stop counter
    lastInfo = QGeoPositionInfo();

    emit iconChanged();
    emit statusChanged();
    emit update();
}


auto Navigation::SatNav::timestampAsString() const -> QString
{
    if (!lastInfo.isValid()) {
        return QStringLiteral("-");
    }
    QDateTime timeStamp = lastInfo.timestamp();
    if (!timeStamp.isValid()) {
        return QStringLiteral("-");
    }

    return timeStamp.time().toString(QStringLiteral("HH:mm:ss")) + " UTC";
}


auto Navigation::SatNav::track() const -> int
{
    if (groundSpeedInKnots() < 4) {
        return -1;
    }
    if (!lastInfo.hasAttribute(QGeoPositionInfo::Direction)) {
        return -1;
    }

    auto track = lastInfo.attribute(QGeoPositionInfo::Direction);
    if (!qIsFinite(track)) {
        return -1;
    }

    auto intTrack = static_cast<int>(qFloor(track+0.5));
    if ((intTrack < 0) || (intTrack > 360)) {
        return -1;
    }

    return intTrack % 360;
}


auto Navigation::SatNav::trackAsString() const -> QString
{
    auto _track = track();

    if (_track < 0) {
        return QStringLiteral("-");
    }
    return QStringLiteral("%1°").arg(_track);
}


auto Navigation::SatNav::wayTo(const QGeoCoordinate& position) const -> QString
{
    // Paranoid safety checks
    if (status() != SatNav::OK) {
        return QString();
    }
    if (!position.isValid()) {
        return QString();
    }
    if (!_lastValidCoordinate.isValid()) {
        return QString();
    }

    auto dist = AviationUnits::Distance::fromM(_lastValidCoordinate.distanceTo(position));
    auto QUJ = qRound(_lastValidCoordinate.azimuthTo(position));

    if (GlobalSettings::useMetricUnitsStatic()) {
        return QStringLiteral("DIST %1 km • QUJ %2°").arg(dist.toKM(), 0, 'f', 1).arg(QUJ);
    }
    return QStringLiteral("DIST %1 NM • QUJ %2°").arg(dist.toNM(), 0, 'f', 1).arg(QUJ);
}
