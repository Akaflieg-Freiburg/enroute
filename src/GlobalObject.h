/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

#pragma once

#include <QObject>

class DemoRunner;
class GlobalSettings;
class Librarian;
class QNetworkAccessManager;
class Sensors;

namespace DataManagement
{
class DataManager;
class SSLErrorHandler;
} // namespace DataManagement

namespace GeoMaps
{
class GeoMapProvider;
class WaypointLibrary;
} // namespace GeoMaps

namespace Navigation
{
class Clock;
class Navigator;
} // namespace Navigation

namespace NOTAM
{
class NotamProvider;
} // namespace NOTAM

namespace Notifications
{
class NotificationManager;
} // namespace Notifications

namespace Traffic
{
class FlarmnetDB;
class PasswordDB;
class TrafficDataProvider;
} // namespace Traffic

namespace Platform
{
class FileExchange_Abstract;
class Notifier_Abstract;
class PlatformAdaptor_Abstract;
} // namespace Platform

namespace Positioning
{
class PositionProvider;
} // namespace Positioning

namespace Weather
{
class WeatherDataProvider;
} // namespace Weather

/*! \brief Base class for global singleton objects
 *
 * This is the base class for static instances of classes that are used
 * throughout the application.  The instances are constructed lazily at runtime,
 * whenever the appropriate methods are called.  They are children of the
 * QCoreApplication object and deleted along with this object.
 *
 * Although all relevant methods are static, it is possible to construct an
 * instance of this class, which allows to use this class from QML.
 *
 * The static methods return pointers to application-wide static objects. They
 * must only be called while a global QCoreApplication instance exists.  If
 * these conditions are satisfied, the pointers returned are guaranteed to be
 * valid.  The instances are owned by this class and must not be deleted. QML
 * ownership has been set to QQmlEngine::CppOwnership.
 *
 * Objects that inherit from this class MUST NOT call any of the static methods
 * from their constructors. Instead, the method deferredInitialization() can be
 * used, which is called immediately after the constructor returns.
 *
 * The methods in this class are reentrant, but not thread safe.
 */

class GlobalObject : public QObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit GlobalObject(QObject *parent = nullptr);

    /*! \brief Standard deconstructor
     *
     * This destructor will destruct all application-wide static instances
     * managed by this class.
     */
    ~GlobalObject() override = default;

    /*! \brief Deletes all globally defined objects
     *
     * This method will delete all globally defined objects in the correct order.
     */
    static void clear();

    /*! \brief Indicates if the static methods are ready to be used
     *
     *  This is relevant for C++ code that is called from Android, often at
     *  unexpected times (during startup, …). This code should check that the
     *  GlobalObject class is ready before using it.
     *
     *  @returns False if the app is in constructing state where the
     *  pointer-returning methods should not be used.
     */
    Q_INVOKABLE static bool canConstruct();

    /*! \brief Pointer to appplication-wide static Navigation::Clock instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static Navigation::Clock* clock();

    /*! \brief Pointer to appplication-wide static GeoMaps::DataManager instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static DataManagement::DataManager* dataManager();

    /*! \brief Pointer to appplication-wide static DemoRunner instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static DemoRunner* demoRunner();

    /*! \brief Pointer to appplication-wide static FlarmnetDB instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static Traffic::FlarmnetDB* flarmnetDB();

    /*! \brief Pointer to appplication-wide static FileExchange instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static Platform::FileExchange_Abstract* fileExchange();

    /*! \brief Pointer to appplication-wide static GeoMaps::GeoMapProvider
     * instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static GeoMaps::GeoMapProvider* geoMapProvider();

    /*! \brief Pointer to appplication-wide static Settings instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static GlobalSettings* globalSettings();

    /*! \brief Pointer to appplication-wide static librarian instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static Librarian* librarian();

    /*! \brief Pointer to appplication-wide static PlatformAdaptor instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static Platform::PlatformAdaptor_Abstract* platformAdaptor();

    /*! \brief Pointer to appplication-wide static Navigation::Clock instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static Sensors* sensors();

    /*! \brief Pointer to appplication-wide static Navigation::Navigator instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static Navigation::Navigator* navigator();

    /*! \brief Pointer to appplication-wide static PasswordDB instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static Traffic::PasswordDB* passwordDB();

    /*! \brief Pointer to appplication-wide static PositionProvider instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static Positioning::PositionProvider* positionProvider();

    /*! \brief Pointer to appplication-wide static notification manager instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static NOTAM::NotamProvider* notamProvider();

    /*! \brief Pointer to appplication-wide static notification manager instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static Notifications::NotificationManager* notificationManager();

    /*! \brief Pointer to appplication-wide static QNetworkAccessManager instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static QNetworkAccessManager* networkAccessManager();

    /*! \brief Pointer to appplication-wide static QNetworkAccessManager instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static DataManagement::SSLErrorHandler* sslErrorHandler();

    /*! \brief Pointer to appplication-wide static TrafficDataProvider instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static Traffic::TrafficDataProvider* trafficDataProvider();

    /*! \brief Pointer to appplication-wide static WaypointLibrary instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static GeoMaps::WaypointLibrary* waypointLibrary();

    /*! \brief Pointer to appplication-wide static WeatherDataProvider instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static Weather::WeatherDataProvider* weatherDataProvider();

protected:
    /*! \brief Non-constructor initialization
     *
     *  This method is called by the static methods that create global instances
     *  immediately after the constructor returns. This class can be
     *  re-implemented to perform initialization steps that refer to other
     *  singleton objects.
     */
    virtual void deferredInitialization()
    {
        ;
    }

private:
    Q_DISABLE_COPY_MOVE(GlobalObject)

    template <typename T>
    static auto allocateInternal(QPointer<T> &pointer) -> T *;
};
