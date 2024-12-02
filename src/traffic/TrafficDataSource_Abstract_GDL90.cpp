/***************************************************************************
 *   Copyright (C) 2021-2024 by Stefan Kebekus                             *
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

#include <array>

#include "GlobalObject.h"
#include "positioning/Geoid.h"
#include "positioning/PositionProvider.h"
#include "traffic/TrafficDataSource_Abstract.h"

using namespace Qt::Literals::StringLiterals;


const std::array<quint16, 256> Crc16Table =
{ 0, 4129, 8258, 12387, 16516, 20645, 24774, 28903, 33032, 37161,
  41290, 45419, 49548, 53677, 57806, 61935, 4657, 528, 12915,
  8786, 21173, 17044, 29431, 25302, 37689, 33560, 45947, 41818,
  54205, 50076, 62463, 58334, 9314, 13379, 1056, 5121, 25830,
  29895, 17572, 21637, 42346, 46411, 34088, 38153, 58862, 62927,
  50604, 54669, 13907, 9842, 5649, 1584, 30423, 26358, 22165,
  18100, 46939, 42874, 38681, 34616, 63455, 59390, 55197, 51132,
  18628, 22757, 26758, 30887, 2112, 6241, 10242, 14371, 51660,
  55789, 59790, 63919, 35144, 39273, 43274, 47403, 23285, 19156,
  31415, 27286, 6769, 2640, 14899, 10770, 56317, 52188, 64447,
  60318, 39801, 35672, 47931, 43802, 27814, 31879, 19684, 23749,
  11298, 15363, 3168, 7233, 60846, 64911, 52716, 56781, 44330,
  48395, 36200, 40265, 32407, 28342, 24277, 20212, 15891, 11826,
  7761, 3696, 65439, 61374, 57309, 53244, 48923, 44858, 40793,
  36728, 37256, 33193, 45514, 41451, 53516, 49453, 61774, 57711,
  4224, 161, 12482, 8419, 20484, 16421, 28742, 24679, 33721,
  37784, 41979, 46042, 49981, 54044, 58239, 62302, 689, 4752,
  8947, 13010, 16949, 21012, 25207, 29270, 46570, 42443, 38312,
  34185, 62830, 58703, 54572, 50445, 13538, 9411, 5280, 1153,
  29798, 25671, 21540, 17413, 42971, 47098, 34713, 38840, 59231,
  63358, 50973, 55100, 9939, 14066, 1681, 5808, 26199, 30326,
  17941, 22068, 55628, 51565, 63758, 59695, 39368, 35305, 47498,
  43435, 22596, 18533, 30726, 26663, 6336, 2273, 14466, 10403,
  52093, 56156, 60223, 64286, 35833, 39896, 43963, 48026, 19061,
  23124, 27191, 31254, 2801, 6864, 10931, 14994, 64814, 60687,
  56684, 52557, 48554, 44427, 40424, 36297, 31782, 27655, 23652,
  19525, 15522, 11395, 7392, 3265, 61215, 65342, 53085, 57212,
  44955, 49082, 36825, 40952, 28183, 32310, 20053, 24180, 11923,
  16050, 3793, 7920 };


// Static Helper functions

auto pInfoFromOwnshipReport(const QByteArray &decodedData) -> QGeoPositionInfo
{
    // Check message size
    if (decodedData.length() != 27) {
        return {};
    }

    // Find latitude
    auto la0 = static_cast<quint8>(decodedData.at(4));
    auto la1 = static_cast<quint8>(decodedData.at(5));
    auto la2 = static_cast<quint8>(decodedData.at(6));
    qint32 laInt = (la0 << 16) + (la1 << 8) + la2;
    if (laInt > 8388607) {
        laInt -= 16777216;
    }
    double const lat = (180.0 / 0x800000) * laInt;

    // Find longitude
    auto ln0 = static_cast<quint8>(decodedData.at(7));
    auto ln1 = static_cast<quint8>(decodedData.at(8));
    auto ln2 = static_cast<quint8>(decodedData.at(9));
    qint32 lnInt = (ln0 << 16) + (ln1 << 8) + ln2;
    if (lnInt > 8388607) {
        lnInt -= 16777216;
    }
    double const lon = (180.0 / 0x800000) * lnInt;

    // Construct coordinate, generate position info
    QGeoCoordinate const coordinate(lat, lon);
    if (!coordinate.isValid()) {
        return {};
    }
    QGeoPositionInfo pInfo(coordinate, QDateTime::currentDateTimeUtc());

    // Find Navigation Accuracy Category for Position
    auto a = static_cast<quint8>(decodedData.at(12)) & 0x0FU;
    switch (a) {
    case 1:
        pInfo.setAttribute(QGeoPositionInfo::HorizontalAccuracy, Units::Distance::fromNM(10.0).toM() );
        break;
    case 2:
        pInfo.setAttribute(QGeoPositionInfo::HorizontalAccuracy, Units::Distance::fromNM(4.0).toM() );
        break;
    case 3:
        pInfo.setAttribute(QGeoPositionInfo::HorizontalAccuracy, Units::Distance::fromNM(2.0).toM() );
        break;
    case 4:
        pInfo.setAttribute(QGeoPositionInfo::HorizontalAccuracy, Units::Distance::fromNM(1.0).toM() );
        break;
    case 5:
        pInfo.setAttribute(QGeoPositionInfo::HorizontalAccuracy, Units::Distance::fromNM(0.5).toM() );
        break;
    case 6:
        pInfo.setAttribute(QGeoPositionInfo::HorizontalAccuracy, Units::Distance::fromNM(0.3).toM() );
        break;
    case 7:
        pInfo.setAttribute(QGeoPositionInfo::HorizontalAccuracy, Units::Distance::fromNM(0.1).toM() );
        break;
    case 8:
        pInfo.setAttribute(QGeoPositionInfo::HorizontalAccuracy, Units::Distance::fromNM(0.05).toM() );
        break;
    case 9:
        pInfo.setAttribute(QGeoPositionInfo::HorizontalAccuracy, 30.0 );
        break;
    case 10:
        pInfo.setAttribute(QGeoPositionInfo::HorizontalAccuracy, 10.0 );
        break;
    case 11:
        pInfo.setAttribute(QGeoPositionInfo::HorizontalAccuracy, 3.0 );
        break;
    default:
        break;
    }

    // Find horizontal speed if available
    auto hh0 = static_cast<quint8>(decodedData.at(13));
    auto hh1 = static_cast<quint8>(decodedData.at(14));
    quint32 const hhTmp = (hh0 << 4) + (hh1 >> 4);
    if (hhTmp != 0xFFF) {
        Units::Speed const hSpeed = Units::Speed::fromKN(hhTmp);
        pInfo.setAttribute(QGeoPositionInfo::GroundSpeed, hSpeed.toMPS() );
    }

    // Find vertical speed if available
    auto vv0 = static_cast<quint8>(decodedData.at(14)) & 0x0FU;
    auto vv1 = static_cast<quint8>(decodedData.at(15));
    qint32 const vvTmp = (vv0 << 8) + vv1;
    if (vvTmp != 0x800) {
        Units::Speed vSpeed;
        if (vvTmp < 0x800) {
            vSpeed = Units::Speed::fromFPM(64.0*vvTmp);
        } else {
            vSpeed = Units::Speed::fromFPM(-64.0*((1<<12)-vvTmp));
        }
        pInfo.setAttribute(QGeoPositionInfo::VerticalSpeed, vSpeed.toMPS() );
    }

    // Find true track if available
    auto mm0 = static_cast<quint8>(decodedData.at(11)) & 0x03U;
    if (mm0 == 1)  {
        auto tt = static_cast<quint8>(decodedData.at(16));
        pInfo.setAttribute(QGeoPositionInfo::Direction, tt*360.0/256.0 );
    }

    return pInfo;
}


// Member functions

void Traffic::TrafficDataSource_Abstract::processGDLMessage(const QByteArray& rawMessage)
{

    //
    // Do some trivial consistency checks
    //

    if (rawMessage.size() < 3) {
        return;
    }


    //
    // Escape character decoding.
    //
    QByteArray message;
    {
        message.reserve(rawMessage.size());
        bool isEscaped = false;
        foreach(auto byte, rawMessage) {
            if (byte == 0x7d) {
                isEscaped = true;
                continue;
            }
            if (isEscaped) {
                message.append( static_cast<char>(static_cast<quint8>(byte) ^ static_cast<quint8>('\x20')) );
                isEscaped = false;
                continue;
            }
            message.append(byte);
        }
        if (isEscaped) {
            return;
        }
    }


    //
    // CRC Checksum verification
    //
    {
        quint16 crc = 0;
        foreach(auto byte, message.chopped(2)) {
            crc = Crc16Table.at(crc >> 8U) ^ (crc << 8U) ^ static_cast<quint8>(byte);
        }

        // Extract CRC checksum from data
        quint16 savedCRC = 0;
        savedCRC += static_cast<quint8>( message.at(message.size()-1) );
        savedCRC = (savedCRC << 8U) + static_cast<quint8>( message.at(message.size()-2) );
        if (crc != savedCRC) {
            return;
        }
    }


    // Extract Message ID, cut off Message ID and checksum from decodedData
    auto messageID = static_cast<quint8>( message.at(0) );
    message.chop(2);
    message = message.mid(1);


    //
    // Handle the various message types
    //

    // Heartbeat message
    if (messageID == 0) {
        if (message.length() < 3) {
            return;
        }

        // Handle runtime errors
        QStringList results;
        auto status = static_cast<quint8>(message.at(0));
        if ((status & 1<<7) == 0) {
            results += tr("No GPS reception");
        }
        if ((status & 1<<6) != 0) {
            results += tr("Maintenance required");
        }
        if ((status & 1<<3) != 0) {
            results += tr("GPS Battery low voltage");
        }
        setTrafficReceiverRuntimeError(results.join(QStringLiteral(" • ")));

        setReceivingHeartbeat(true);
        return;
    }

    // Ownship report
    if (messageID == 10) {
        // Get position info w/o altitude information
        auto pInfo = pInfoFromOwnshipReport(message);
        if (!pInfo.isValid()) {
            return;
        }

        // Copy true altitude into pInfo, if known
        if (m_trueAltitudeTimer.isActive()) {
            auto coordinate = pInfo.coordinate();
            coordinate.setAltitude(m_trueAltitude.toM());
            pInfo.setCoordinate(coordinate);
            pInfo.setAttribute(QGeoPositionInfo::VerticalAccuracy, m_trueAltitudeFOM.toM() );
        }

        // Find pressure altitude and update information if need be
        auto dd0 = static_cast<quint8>(message.at(10));
        auto dd1 = static_cast<quint8>(message.at(11));
        quint32 const ddTmp = (dd0 << 4) + (dd1 >> 4);
        if (ddTmp != 0xFFF) {
            m_pressureAltitude = Units::Distance::fromFT(25.0*ddTmp - 1000.0);
            m_pressureAltitudeTimer.start();
        } else {
            m_pressureAltitude = Units::Distance::fromM( qQNaN() );
            m_pressureAltitudeTimer.stop();
        }
        setPressureAltitude(m_pressureAltitude);

        // Update position information and continue
        emit positionUpdated( Positioning::PositionInfo(pInfo) );
        return;
    }

    // Ownship geometric altitude
    if (messageID == 11) {
        // Find geometric alt and apply geoid correction
        auto dd0 = static_cast<quint8>(message.at(0));
        auto dd1 = static_cast<quint8>(message.at(1));
        qint32 ddInt = (dd0 << 8) + dd1;
        if (ddInt > 32767) {
            ddInt -= 65536;
        }
        m_trueAltitude = Units::Distance::fromFT(ddInt*5.0);
        auto geoidCorrection = Positioning::Geoid::separation( Positioning::PositionProvider::lastValidCoordinate() );
        if (geoidCorrection.isFinite()) {
            m_trueAltitude = m_trueAltitude-geoidCorrection;
        }

        // Find geometric figure of merit
        auto vm0 = static_cast<quint8>(message.at(2)) & 0x7FU;
        auto vm1 = static_cast<quint8>(message.at(3));
        auto vmInt = (vm0 << 8) + vm1;
        m_trueAltitudeFOM = Units::Distance::fromM(vmInt);
        m_trueAltitudeTimer.start();
    }

    // Traffic report
    if (messageID == 20) {

        // Get position info w/o altitude information
        auto pInfo = pInfoFromOwnshipReport(message);
        if (!pInfo.isValid()) {
            return;
        }

        // Get ID
        auto id0 = static_cast<quint8>(message.at(0)) & 0x0FU;
        auto id1 = static_cast<quint8>(message.at(1));
        auto id2 = static_cast<quint8>(message.at(2));
        auto id3 = static_cast<quint8>(message.at(3));
        auto id = QString::number(id0, 16) + QString::number(id1, 16) + QString::number(id2, 16) + QString::number(id3, 16);

        // Alert
        auto s0 = static_cast<quint8>(message.at(0)) >> 4;
        auto alert = (s0 == 1) ? 1 : 0;

        // Traffic type
        auto ee = static_cast<quint8>(message.at(17));
        auto type = Traffic::TrafficFactor_Abstract::unknown;
        switch(ee) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            type = Traffic::TrafficFactor_Abstract::Aircraft;
            break;
        case 6:
            type = Traffic::TrafficFactor_Abstract::Jet;
            break;
        case 7:
            type = Traffic::TrafficFactor_Abstract::Copter;
            break;
        case 9:
            type = Traffic::TrafficFactor_Abstract::Glider;
            break;
        case 10:
            type = Traffic::TrafficFactor_Abstract::Balloon;
            break;
        case 11:
            type = Traffic::TrafficFactor_Abstract::Skydiver;
            break;
        case 14:
            type = Traffic::TrafficFactor_Abstract::Drone;
            break;
        case 19:
            type = Traffic::TrafficFactor_Abstract::StaticObstacle;
            break;
        default:
            break;
        }

        // Compute true altitude and altitude distance of traffic if
        // a recent pressure altitude reading for owncraft exists.
        Units::Distance vDist {};
        if (m_pressureAltitudeTimer.isActive()) {
            auto dd0 = static_cast<quint8>(message.at(10));
            auto dd1 = static_cast<quint8>(message.at(11));
            quint32 const ddTmp = (dd0 << 4) + (dd1 >> 4);
            if (ddTmp != 0xFFF) {
                auto trafficPressureAltitude = Units::Distance::fromFT(25.0*ddTmp - 1000.0);
                vDist = trafficPressureAltitude - m_pressureAltitude;

                // Compute true altitude of traffic if possible
                if (m_trueAltitudeTimer.isActive()) {
                    auto trafficTrueAltitude = m_trueAltitude + vDist;
                    auto coordinate = pInfo.coordinate();
                    coordinate.setAltitude(trafficTrueAltitude.toM());
                    pInfo.setCoordinate(coordinate);
                }
            }
        }

        // Compute horizontal distance to traffic if our own position
        // is known.
        Units::Distance hDist {};
        auto* positionProviderPtr = GlobalObject::positionProvider();
        if (positionProviderPtr != nullptr) {
            auto ownShipCoordinate = positionProviderPtr->positionInfo().coordinate();
            auto trafficCoordinate = pInfo.coordinate();
            if (ownShipCoordinate.isValid() && trafficCoordinate.isValid()) {
                hDist = Units::Distance::fromM( ownShipCoordinate.distanceTo(trafficCoordinate) );
            }
        }

        // Callsign of traffic
        auto callSign = QString::fromLatin1(message.mid(18,8)).simplified();

        // Expose data
        if ((callSign.compare(u"MODE S"_s, Qt::CaseInsensitive) == 0) || (callSign.compare(u"MODE-S"_s, Qt::CaseInsensitive) == 0)) {
            m_factorDistanceOnly.setAlarmLevel(alert);
            m_factorDistanceOnly.setCallSign(callSign);
            m_factorDistanceOnly.setCoordinate(Positioning::PositionProvider::lastValidCoordinate());
            m_factorDistanceOnly.setHDist(hDist);
            m_factorDistanceOnly.setID(id);
            m_factorDistanceOnly.setType(type);
            m_factorDistanceOnly.setVDist(vDist);
            m_factorDistanceOnly.startLiveTime();
            emit factorWithoutPosition(m_factorDistanceOnly);
        } else {
            m_factor.setAlarmLevel(alert);
            m_factor.setCallSign(callSign);
            m_factor.setHDist(hDist);
            m_factor.setID(id);
            m_factor.setPositionInfo( Positioning::PositionInfo(pInfo) );
            m_factor.setType(type);
            m_factor.setVDist(vDist);
            m_factor.startLiveTime();
            emit factorWithPosition(m_factor);
        }
    }

}

