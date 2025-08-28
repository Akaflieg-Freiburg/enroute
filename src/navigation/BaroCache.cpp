
#include "BaroCache.h"
#include "positioning/PositionProvider.h"
#include "traffic/TrafficDataProvider.h"

Navigation::BaroCache::BaroCache(QObject* parent)
    : QObject(parent)
{
    qWarning() << "BaroCache constructor";

    notifiers.push_back(GlobalObject::positionProvider()->bindablePositionInfo().addNotifier([this]() {
        const auto geometricAltitude = GlobalObject::positionProvider()->positionInfo().trueAltitudeAMSL();
        qWarning() << "Geometric Altitude Received" << geometricAltitude.toM();
        if (!geometricAltitude.isFinite() || (geometricAltitude.toM() < -1000) || (geometricAltitude.toM() > 10000))
        {
            return;
        }
        m_incomingGeometricAltitude = geometricAltitude;
        m_incomingGeometricAltitudeTimestamp = QDateTime::currentDateTime();
        addIncomingBaroCacheData();
    }));
    notifiers.push_back(GlobalObject::trafficDataProvider()->bindablePressureAltitude().addNotifier([this]() {
        const auto pressureAltitude = GlobalObject::trafficDataProvider()->pressureAltitude();
        qWarning() << "Pressure Altitude Received" << pressureAltitude.toM();
        if (!pressureAltitude.isFinite() || (pressureAltitude.toM() < -1000) || (pressureAltitude.toM() > 10000))
        {
            return;
        }
        m_incomingPressureAltitude = pressureAltitude;
        m_incomingPressureAltitudeTimestamp = QDateTime::currentDateTime();
        addIncomingBaroCacheData();
    }));

#warning need to clean data every 20mins or so
#warning might want to save data every few minutes for later recovery, if app goes off by accident
}


void Navigation::BaroCache::addIncomingBaroCacheData()
{
    qWarning() << "addIncomingBaroCacheData";
    // Update data
    if (!m_incomingPressureAltitude.isFinite() || !m_incomingGeometricAltitudeTimestamp.isValid()
        || !m_incomingGeometricAltitude.isFinite() || !m_incomingGeometricAltitudeTimestamp.isValid()
        || (qAbs(m_incomingGeometricAltitudeTimestamp.msecsTo(m_incomingPressureAltitudeTimestamp)) > TIME_WINDOW))
    {
        return;
    }

    qWarning() << m_incomingPressureAltitudeTimestamp << m_incomingPressureAltitude.toM() << m_incomingGeometricAltitude.toM();
    auto FL = qRound(m_incomingPressureAltitude.toFeet()/100.0);
    m_altitudeElementsByFlightLevel[FL] = {m_incomingPressureAltitudeTimestamp, m_incomingPressureAltitude, m_incomingGeometricAltitude};
}


Units::Distance Navigation::BaroCache::estimatedGeometricAltitude(Units::Distance pressureAltitude, bool allowImpreciseEstimate)
{

#warning Implement
    return {};
/*
    std::vector<double> pressureAltitudeList;
    std::vector<double> geometricAltitudeList;
    for(int i = 0; i < m_altitudeElements.size(); i++)
    {
        if((m_altitudeElements.at(i).pressureAltitude.toM() > 0.0) && (m_altitudeElements.at(i).geometricAltitude.toM() > 0.0))
        {
            pressureAltitudeList.push_back(m_altitudeElements.at(i).pressureAltitude.toM());
            geometricAltitudeList.push_back(m_altitudeElements.at(i).geometricAltitude.toM());
        }
    }
    tk::spline s(pressureAltitudeList, geometricAltitudeList);
    double value = s(pressureAltitude.toM());
    return Units::Distance::fromM(value);
*/
}
