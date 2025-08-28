
#pragma once

#include <QGeoPositionInfo>
#include <QPropertyNotifier>
#include <QTime>

#include "positioning/PositionInfo.h"
#include "units/Distance.h"


namespace Navigation
{

/*! \brief Cache relating geometric and barometric altitude information
 *
 * This class collects geometric and barometric altitude information of the own aircraft. If sufficient data is available, it uses this data to
 * estimate geometric altitudes from barometric altitudes and vice versa. This functionality is used, for instance, to estimate the geometric
 * altitude of airspace boundaries (which are defined as geometric altitudes).
 *
 * For clarity: We use the term "barometric altitude" to refer to the baromatric altitude over the standard level. This is the altitude shown
 * by an aircraft altimeter when set to 1013.2 hPa.
 */
class BaroCache : public QObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    BaroCache(QObject* parent = nullptr);

    // Default destructor
    ~BaroCache() override = default;



    //
    // Methods
    //

    /*! \brief Estimate geometric altitude for a given pressure altitude
     *
     *  This method queries the cache, in order to estimate the geometric altitude for a given pressure altitude.
     *  If allowImpreciseEstimate is false, it will return an invalid Distance unless there exists a cache entry whose pressure
     *  altitude is within +/- 500ft from the pressureAltitude parameter.
     *
     *  Remember: pressure altitude is the barometric altitude above the 1013.2 hPa pressure surface
     *
     *  @param pressureAltitude Pressure altitude whose associated geometric altitude is to be estimated
     *
     *  @param allowImpreciseEstimate Return an estimate even if the cache is empty or contains little data.
     *
     *  @returns Estimated geometric altitude
     */
    Units::Distance estimatedGeometricAltitude(Units::Distance pressureAltitude, bool allowImpreciseEstimate=false);

private:
    Q_DISABLE_COPY_MOVE(BaroCache)

    void addIncomingBaroCacheData();


    // Incoming barometric and geometric altitudes are considered related if their time stamps differ at most be 20ms.
    static constexpr auto TIME_WINDOW = 20U;

    Units::Distance m_incomingGeometricAltitude;
    QDateTime m_incomingGeometricAltitudeTimestamp;

    Units::Distance m_incomingPressureAltitude;
    QDateTime m_incomingPressureAltitudeTimestamp;

    struct altitudeElement
    {
        QDateTime timestamp;
        Units::Distance pressureAltitude;
        Units::Distance geometricAltitude;
    };

    QMap<int, altitudeElement> m_altitudeElementsByFlightLevel;

    std::vector<QPropertyNotifier> notifiers;
};

} // namespace Navigation
