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
class Librarian;
class MobileAdaptor;
class QNetworkAccessManager;
class Settings;

namespace DataManagement {
class DataManager;
}

namespace GeoMaps {
class GeoMapProvider;
};

namespace Navigation {
class Navigator;
}

namespace Traffic {
class FlarmnetDB;
class PasswordDB;
class TrafficDataProvider;
}

namespace Platform {
class Notifier;
}

namespace Positioning {
class PositionProvider;
}

namespace Weather {
class WeatherDataProvider;
}

#warning docu!

/*! \brief GlobalObject instance storage
 *
 * This class manages a collection of static instances of classes that are used
 * throughout the application.  The instances are constructed lazily at runtime,
 * whenever the appropriate methods are called.  They are children of the GlobalObject
 * QCoreApplication object and deleted along with this object.
 *
 * Although all relevant methods are static, it is possible to construct an
 * instance of this class, which allows to use this class from QML.
 *
 * The static methods return pointers to application-wide static objects. These
 * pointer is guaranteed to be valid.  The instances are owned by this class and
 * must not be deleted. QML ownership has been set to QQmlEngine::CppOwnership.
 *
 * @note This method must only be called while a GlobalObject QCoreApplication exists.
 * If GlobalObject manages an instance of your class, then none of the static methods
 * must not be called in the constructor of your class, or else an inite loop
 * may result.
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
    ~GlobalObject() = default;

#warning docu
    virtual void deferredInitialization()
    {
        ;
    }

    /*! \brief Indicates if this class is ready to be used
     *
     *  This method returns false if the app is in constructing state
     *  where the pointer-returning methods should not be used.
     *
     *  This is relevant for C++ code that is called from Android, often at
     *  unexpected times (during startup, …). This code should check that
     *  the GlobalObject class is ready before using it.
     */
#warning find better name
    Q_INVOKABLE static bool ready();

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

    /*! \brief Pointer to appplication-wide static GeoMaps::GeoMapProvider instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static GeoMaps::GeoMapProvider* geoMapProvider();

    /*! \brief Pointer to appplication-wide static librarian instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static Librarian* librarian();

    /*! \brief Pointer to appplication-wide static MobileAdaptor instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static MobileAdaptor* mobileAdaptor();

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
    Q_INVOKABLE static Platform::Notifier* notifier();

    /*! \brief Pointer to appplication-wide static QNetworkAccessManager instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static QNetworkAccessManager* networkAccessManager();

    /*! \brief Pointer to appplication-wide static Settings instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static Settings* settings();

    /*! \brief Pointer to appplication-wide static TrafficDataProvider instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static Traffic::TrafficDataProvider* trafficDataProvider();

    /*! \brief Pointer to appplication-wide static WeatherDataProvider instance
     *
     * @returns Pointer to appplication-wide static instance.
     */
    Q_INVOKABLE static Weather::WeatherDataProvider* weatherDataProvider();

private:
    Q_DISABLE_COPY_MOVE(GlobalObject)

    template<typename T> static T* allocateInternal(QPointer<T>& pointer);
};
