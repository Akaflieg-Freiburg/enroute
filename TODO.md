# Appearances of qRound

navigation/Aircraft.cpp:    auto QUJ = qRound(from.azimuthTo(to));
navigation/Aircraft.cpp:                    result += QStringLiteral("E %1").arg(qRound(E));
navigation/Aircraft.cpp:        return QStringLiteral("%L1 kn").arg(qRound( speed.toKN() ));
navigation/Aircraft.cpp:        return QStringLiteral("%L1 km/h").arg(qRound( speed.toKMH() ));
navigation/Aircraft.cpp:        return QStringLiteral("%L1 mph").arg(qRound( speed.toMPH() ));
navigation/Aircraft.cpp:        return signString+QStringLiteral("%L1 ft").arg(qRound(distance.toFeet()));
navigation/Aircraft.cpp:        return signString+QStringLiteral("%L1 m").arg(qRound(distance.toM()));
navigation/Aircraft.cpp:        return QStringLiteral("%L1 ft/min").arg(qRound( speed.toFPM() ));
navigation/Aircraft.cpp:        return QStringLiteral("%L1 m/s").arg(qRound( speed.toMPS() ));
navigation/Aircraft.cpp:        return QStringLiteral("%L1 l").arg(qRound( volume.toL() ));
navigation/Aircraft.cpp:        return QStringLiteral("%L1 gal").arg(qRound( volume.toGAL() ));
navigation/Navigator.cpp:        rri.nextWP_ETA = QDateTime::currentDateTimeUtc().addSecs( qRound64(rri.nextWP_ETE.toS()) );
navigation/Navigator.cpp:            rri.finalWP_ETA = QDateTime::currentDateTimeUtc().addSecs( qRound64(rri.finalWP_ETE.toS()) ).toUTC();
notam/NOTAMProvider.cpp:    const QGeoCoordinate coordinateRounded( qRound(coordinate.latitude()), qRound(coordinate.longitude()) );
notam/NOTAMProvider.cpp:                         .arg( qRound(requestRadius.toNM()) );
traffic/TrafficDataSource_Abstract_FLARM.cpp:        time = time.addMSecs(qRound(MS.toDouble()*1000.0));
traffic/TrafficDataSource_Ogn.cpp:                            .arg(qRound(m_receiveRadius.toKM()));
traffic/TrafficDataSource_Ogn.cpp:                            .arg(qRound(m_receiveRadius.toKM()));
traffic/Warning.cpp:            result << QCoreApplication::translate("Traffic::Warning", "Distance %1 km").arg(qRound(m_hDist.toKM()*10.0)/10.0);
traffic/Warning.cpp:            result << QCoreApplication::translate("Traffic::Warning", "Distance %1 nm").arg(qRound(m_hDist.toNM()*10.0)/10.0);
traffic/Warning.cpp:            result << QCoreApplication::translate("Traffic::Warning", "Distance %1 mil").arg(qRound(m_hDist.toMIL()*10.0)/10.0);
ui/ScaleQuickItem.cpp:    int const sizeOfUnitInPix = qRound(ScaleUnitInUnit * pixelPerUnit);
ui/ScaleQuickItem.cpp:    int const sizeOfScaleInPix = qRound(sizeOfScaleInUnit * pixelPerUnit);
ui/ScaleQuickItem.cpp:        font.setPixelSize(qRound(font.pixelSize()*0.8));
ui/ScaleQuickItem.cpp:    int const baseX = m_vertical ? 8 : qRound((width() - sizeOfScaleInPix) / 2.0);
ui/ScaleQuickItem.cpp:    int const baseY = m_vertical ? qRound((height() - sizeOfScaleInPix) / 2.0)
ui/ScaleQuickItem.cpp:                                : qRound(height()) - 8;
ui/ScaleQuickItem.cpp:        painter->drawText(-qRound(height()/2.0)-textWidth/2, baseX+textHeight, text);
ui/SideviewQuickItem.cpp:        m_track = u"Direction → %1°"_s.arg( qRound(ownshipTrack.toDEG()));
ui/SideviewQuickItem.cpp:    xCoordinates.reserve( qRound((width()+20)/step) );
weather/Decoder.cpp:                    results << QStringLiteral("%1 m").arg(qRound(*d));
weather/Decoder.cpp:            results << QString::number(qRound(*d));
weather/Decoder.cpp:                results << QStringLiteral("(%1 m)").arg(qRound(*d));
weather/Decoder.cpp:        return modifier + QStringLiteral("%1 ft").arg(qRound(*d));
weather/Decoder.cpp:        return QStringLiteral("%1 hPa").arg(qRound(*phpa));
weather/Decoder.cpp:            return QStringLiteral("%1 km/h").arg(qRound(*s));
weather/Decoder.cpp:        return QStringLiteral("%1 kt").arg(qRound(*s));
weather/Decoder.cpp:        temperatureString = QStringLiteral("%1 °C").arg(qRound(*t));
weather/Decoder.cpp:                return tr("wave height: %1 m").arg(qRound(*h));
weather/Decoder.cpp:            return tr("Duration of sunshine that occurred the previous calendar day is %1 minutes.").arg(qRound(*duration));
weather/Decoder.cpp:        return tr("Density altitude is %1 feet").arg(qRound(*data));
weather/METAR.cpp:        items += tr("Relative Humidity: %1%").arg(qRound(relativeHumidity));
weather/METAR.cpp:                     + tr("Expect %1\% increase in takeoff distance").arg(qRound(takeoffDistIncPercentage));
weather/METAR.cpp:                         + tr("Expect %1\% decrease in climb rate").arg(qRound(rateOfClimbDecreasePercentage));
weather/WeatherDataProvider.cpp:    auto timeZone = qRound(coord.longitude()/15.0);
weather/WeatherDataProvider.cpp:        sunrise.setTime(QTime::fromMSecsSinceStartOfDay(qRound(sunriseTimeInMin*60*1000)));
weather/WeatherDataProvider.cpp:        sunset.setTime(QTime::fromMSecsSinceStartOfDay(qRound(sunsetTimeInMin*60*1000)));
weather/WeatherDataProvider.cpp:        sunriseTomorrow.setTime(QTime::fromMSecsSinceStartOfDay(qRound(sunriseTomorrowTimeInMin*60*1000)));
weather/WeatherDataProvider.cpp:        return tr("%1 hPa in %2, %3").arg(qRound(closestMETARWithQNH.QNH().toHPa()))


# Appearances of qCeil

geomaps/GeoMapProvider.cpp:    pix = image->pixel(qCeil(intraTileX), qFloor(intraTileY));
geomaps/GeoMapProvider.cpp:    pix = image->pixel(qCeil(intraTileX), qCeil(intraTileY));
geomaps/GeoMapProvider.cpp:    pix = image->pixel(qFloor(intraTileX), qCeil(intraTileY));
navigation/Navigator.cpp:        auto newAltLimit = Units::Distance::fromFT(500.0*qCeil(trueAltitude.toFeet()/500.0+2.0));


# Appearances of qFloor

geomaps/GeoMapProvider.cpp:    auto t = intraTileX - qFloor(intraTileX);
geomaps/GeoMapProvider.cpp:    auto u = intraTileY - qFloor(intraTileY);
geomaps/GeoMapProvider.cpp:    auto pix = image->pixel(qFloor(intraTileX), qFloor(intraTileY));
geomaps/GeoMapProvider.cpp:    pix = image->pixel(qCeil(intraTileX), qFloor(intraTileY));
geomaps/GeoMapProvider.cpp:    pix = image->pixel(qFloor(intraTileX), qCeil(intraTileY));
geomaps/GeoMapProvider.cpp:        const qint64 keyA = qFloor(tilex) & 0xFFFF;
geomaps/GeoMapProvider.cpp:        const qint64 keyB = qFloor(tiley) & 0xFFFF;
geomaps/GeoMapProvider.cpp:            auto tileData = mbtPtr->tile(zoom, qFloor(tilex), qFloor(tiley));
positioning/Geoid.cpp:    int const north = qFloor(row(latitude));
positioning/Geoid.cpp:    int const west = qFloor(col(longitude)) % egm96_cols;
positioning/Geoid.cpp:    double col_dist = col(longitude) - qFloor(col(longitude));
