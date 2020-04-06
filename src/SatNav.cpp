/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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
#include <QtMath>
#include <QVariant>

#include "AviationUnits.h"
#include "SatNav.h"


SatNav::SatNav(QObject *parent)
    : QObject(parent),
      _lastValidCoordinate(EDTF_lat, EDTF_lon, EDTF_ele)
{
    source = QGeoPositionInfoSource::createDefaultSource(this);

    if (source) {
        sourceStatus = source->error();
        connect(source, SIGNAL(error(QGeoPositionInfoSource::Error)), this, SLOT(error(QGeoPositionInfoSource::Error)));
        connect(source, SIGNAL(updateTimeout()), this, SLOT(timeout()));
        connect(source, SIGNAL(positionUpdated(QGeoPositionInfo)), this, SLOT(statusUpdate(QGeoPositionInfo)));
    }

    QSettings settings;
    QGeoCoordinate tmp;
    tmp.setLatitude(settings.value("SatNav/lastValidLatitude", _lastValidCoordinate.latitude()).toDouble());
    tmp.setLongitude(settings.value("SatNav/lastValidLongitude", _lastValidCoordinate.longitude()).toDouble());
    tmp.setAltitude(settings.value("SatNav/lastValidAltitude", _lastValidCoordinate.altitude()).toDouble());
    altitudeCorrectionInM = settings.value("SatNav/altitudeCorrection", 0).toInt();
    if ((tmp.type() == QGeoCoordinate::Coordinate2D) || (tmp.type() == QGeoCoordinate::Coordinate3D))
        _lastValidCoordinate = tmp;
    _lastValidTrack = qBound(0, settings.value("SatNav/lastValidTrack", 0).toInt(), 359);

    if (source) {
        source->startUpdates();
    }


    // Adjust and connect timeoutCounter
    timeoutCounter.setSingleShot(true);
    connect(&timeoutCounter, SIGNAL(timeout()), this, SLOT(timeout()));
}


SatNav::~SatNav()
{
    QSettings settings;

    settings.setValue("SatNav/lastValidLatitude", _lastValidCoordinate.latitude());
    settings.setValue("SatNav/lastValidLongitude", _lastValidCoordinate.longitude());
    settings.setValue("SatNav/lastValidAltitude", _lastValidCoordinate.altitude());
    settings.setValue("SatNav/lastValidTrack", _lastValidTrack);
    settings.setValue("SatNav/altitudeCorrection", altitudeCorrectionInM);
    delete source;
}


int SatNav::altitudeInFeet() const
{
    if (lastInfo.coordinate().type() != QGeoCoordinate::Coordinate3D)
        return 0;

    auto correction = AviationUnits::Distance::fromM(altitudeCorrectionInM);
    return qRound(rawAltitudeInFeet() + correction.toFeet());
}


QString SatNav::altitudeInFeetAsString() const
{
    if (lastInfo.coordinate().type() != QGeoCoordinate::Coordinate3D)
        return "-";

    return myLocale.toString(altitudeInFeet()) + " ft";
}


void SatNav::setAltitudeInFeet(int altitudeInFeet)
{
    if (lastInfo.coordinate().type() != QGeoCoordinate::Coordinate3D)
        return;

    auto altCorrection = AviationUnits::Distance::fromFT(altitudeInFeet-rawAltitudeInFeet());

    altitudeCorrectionInM = qRound(altCorrection.toM());
    QSettings settings;
    settings.setValue("SatNav/altitudeCorrection", altitudeCorrectionInM);
    emit update();
}


int SatNav::rawAltitudeInFeet() const
{
    if (lastInfo.coordinate().type() != QGeoCoordinate::Coordinate3D)
        return 0;

    auto alt = AviationUnits::Distance::fromM(lastInfo.coordinate().altitude());
    return qRound(alt.toFeet() - geoidalSeparation());
}


QString SatNav::rawAltitudeInFeetAsString() const
{
    if (lastInfo.coordinate().type() != QGeoCoordinate::Coordinate3D)
        return "-";

    return myLocale.toString(rawAltitudeInFeet()) + " ft";
}


int SatNav::geoidalSeparation() const
{
    auto separation = AviationUnits::Distance::fromM(geoid());
    return qRound(separation.toFeet());
}


QString SatNav::geoidalSeparationAsString() const
{
    if (!geoid.valid())
        return "-";

    return myLocale.toString(geoidalSeparation()) + " ft";
}


void SatNav::error(QGeoPositionInfoSource::Error newSourceStatus)
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


int SatNav::groundSpeedInKnots() const
{
    if (!lastInfo.isValid())
        return -1;
    if (!lastInfo.hasAttribute(QGeoPositionInfo::GroundSpeed))
        return -1;

    AviationUnits::Speed groundSpeed = AviationUnits::Speed::fromMPS(lastInfo.attribute(QGeoPositionInfo::GroundSpeed));
    if (!groundSpeed.isFinite())
        return -1;
    if (groundSpeed.isNegative())
        return -1;

    return qRound(groundSpeed.toKT());
}


QString SatNav::groundSpeedInKnotsAsString() const
{
    auto _groundSpeedInKnots = groundSpeedInKnots();

    if (_groundSpeedInKnots < 0)
        return "-";
    return myLocale.toString(_groundSpeedInKnots) + " kt";
}


qreal SatNav::groundSpeedInMetersPerSecond() const
{
    if (!lastInfo.isValid())
        return -1.0;
    if (!lastInfo.hasAttribute(QGeoPositionInfo::GroundSpeed))
        return -1.0;

    auto groundSpeed = lastInfo.attribute(QGeoPositionInfo::GroundSpeed);
    if (!qIsFinite(groundSpeed))
        return -1.0;
    if (groundSpeed < 0.0)
        return -1.0;

    return groundSpeed;
}

int SatNav::horizontalPrecisionInMeters() const
{
    if (!lastInfo.isValid())
        return -1;
    if (!lastInfo.hasAttribute(QGeoPositionInfo::HorizontalAccuracy))
        return -1;

    auto horizontalPrecision = lastInfo.attribute(QGeoPositionInfo::HorizontalAccuracy);
    if (!qIsFinite(horizontalPrecision))
        return -1;
    if (horizontalPrecision <= 0.0)
        return -1;

    return static_cast<int>(qFloor(horizontalPrecision+0.5));
}


QString SatNav::horizontalPrecisionInMetersAsString() const
{
    auto _horizontalPrecisionInMeters = horizontalPrecisionInMeters();

    if (_horizontalPrecisionInMeters < 0)
        return "-";
    return QString("±%1 m").arg(_horizontalPrecisionInMeters);
}


QString SatNav::icon() const
{
    if (status() != OK)
        return "/icons/self-noSatNav.svg";

    if (track() < 0)
        return "/icons/self-noDirection.svg";

    return "/icons/self-withDirection.svg";
}


QGeoCoordinate SatNav::lastValidCoordinate() const
{
    return _lastValidCoordinate;
}


QString SatNav::latitudeAsString() const
{
    if (!lastInfo.isValid())
        return "-";

    auto x = lastInfo.coordinate().latitude();
    if ((!qIsFinite(x)) || (x < -90.0) || (x > 90.0))
        return "-";

    auto angle = AviationUnits::Angle::fromDEG(x);
    QString latString = angle.toString();
    latString += (x >= 0) ? QString(" N") : QString(" S");
    return latString;
}


QString SatNav::longitudeAsString() const
{
    if (!lastInfo.isValid())
        return "-";

    auto x = lastInfo.coordinate().longitude();
    if ((!qIsFinite(x)) || (x < -180.0) || (x > 180.0))
        return "-";

    auto angle = AviationUnits::Angle::fromDEG(x);
    QString lonString = angle.toString();

    lonString += (x >= 0) ? QString(" E") : QString(" W");
    return lonString;
}


SatNav::Status SatNav::status() const
{
    if (source == nullptr)
        return Status::Error;

    if (sourceStatus != QGeoPositionInfoSource::NoError)
        return Status::Error;

    if (!timeoutCounter.isActive())
        return Status::Timeout;

    return Status::OK;
}


QString SatNav::statusAsString() const
{
    if (source == nullptr)
        return tr("Not installed or access denied");

    if (sourceStatus == QGeoPositionInfoSource::AccessError)
        return tr("Access denied");

    if (sourceStatus == QGeoPositionInfoSource::ClosedError)
        return tr("Connection to SatNav system lost");

    if (sourceStatus != QGeoPositionInfoSource::NoError)
        return tr("Unknown error");

    if (!timeoutCounter.isActive())
        return tr("Waiting for signal");

    return tr("OK");
}


void SatNav::statusUpdate(const QGeoPositionInfo &info)
{
    // Ignore invalid infos
    if (!info.isValid())
        return;

    // Save old values and set sourceStatus to QGeoPositionInfoSource::NoError
    auto oldStatus = status();
    auto oldIcon   = icon();
    sourceStatus = QGeoPositionInfoSource::NoError;

    lastInfo = info;
    timeoutCounter.start(timeoutThreshold);

    // Inform others
    if (oldStatus != Status::OK)
        emit statusChanged();

    _lastValidCoordinate = info.coordinate();
    emit update();

    // emit iconChanged() if appropriate
    if (oldIcon != icon())
        emit iconChanged();

    // emit lastValidTrackChanged if appropriate
    auto newTrack = track();
    if ((newTrack >= 0) && (newTrack != _lastValidTrack)) {
        _lastValidTrack = newTrack;
        emit lastValidTrackChanged();
    }
}


void SatNav::timeout()
{
    // Clear lastInfo, stop counter
    lastInfo = QGeoPositionInfo();

    emit iconChanged();
    emit statusChanged();
    emit update();
}


QString SatNav::timestampAsString() const
{
    if (!lastInfo.isValid())
        return "-";
    QDateTime timeStamp = lastInfo.timestamp();
    if (!timeStamp.isValid())
        return "-";

    return timeStamp.time().toString("HH:mm:ss") + " UTC";
}


int SatNav::track() const
{
    if (groundSpeedInKnots() < 4)
        return -1;
    if (!lastInfo.hasAttribute(QGeoPositionInfo::Direction))
        return -1;

    auto track = lastInfo.attribute(QGeoPositionInfo::Direction);
    if (!qIsFinite(track))
        return -1;

    auto intTrack = static_cast<int>(qFloor(track+0.5));
    if ((intTrack < 0) || (intTrack > 360))
        return -1;

    return intTrack % 360;
}


QString SatNav::trackAsString() const
{
    auto _track = track();

    if (_track < 0)
        return "-";
    return QString("%1°").arg(_track);
}
