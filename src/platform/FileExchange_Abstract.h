/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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

#include <QQmlEngine>
#include "GlobalObject.h"
#include "geomaps/Waypoint.h"
#include "geomaps/VAC.h"

namespace Platform {

/*! \brief Interface to platform-specific file exchange functionality
 *
 * This pure virtual class is an interface to file exchange functionality that
 * need platform-specific code to operate. The files FileExchange_XXX.(h|cpp)
 * implement a child class FileExchange that contains the actual implementation.
 *
 * If supported by the platform, child classes need to react to requests by the
 * platform to open a file (e.g. a GeoJSON file containing a flight route). Once
 * a request is received, the method processFileRequest() should be called.
 */

class FileExchange_Abstract : public GlobalObject
{
    Q_OBJECT

public:
    /*! \brief Functions and types of a file that this app handles */
    enum FileFunction : quint8
      {
        UnknownFunction, /*< Unknown file */
        FlightRouteOrWaypointLibrary, /*< File contains a flight route or a waypoint library. */
        FlightRoute, /*< File contains a flight route. */
        VectorMap, /*< File contains a vector map. */
        RasterMap, /*< File contains a raster map. */
        WaypointLibrary, /*< Waypoint library in CUP or GeoJSON format */
        OpenAir, /*< Airspace data in openAir format */
        Image, /*< Image without georeferencing information */
        TripKit, /*< Trip Kit */
        ZipFile /*< Zip File */
      };
    Q_ENUM(FileFunction)


    /*! \brief Standard constructor
     *
     * @param parent Standard QObject parent pointer
    */
    explicit FileExchange_Abstract(QObject* parent = nullptr);

    ~FileExchange_Abstract() override = default;



    //
    // Methods
    //


    /*! \brief Import content from file
     *
     * On desktop systems, this method is supposed to open a file dialog to
     * import a file. On mobile systems, this method is supposed to do nothing.
     *
     */
    Q_INVOKABLE virtual void importContent() = 0;

    /*! \brief Share content
     *
     * On desktop systems, this method is supposed to show a file dialog to save
     * the file. On mobile devices, this method is supposed to open a dialog
     * that allows to chose the method to send this file (e-mail, dropbox,
     * signal chat, …)
     *
     * @param content File content
     *
     * @param mimeType the mimeType of the content
     *
     * @param fileNameTemplate A string of the form "EDTF - EDTG", without
     * suffix of path. This can be used, e.g. as the name of the attachment when
     * sending files by e-mail.
     *
     * @returns Empty string on success, the string "abort" on abort, and a
     * translated error message otherwise
     */
    Q_INVOKABLE virtual QString shareContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) = 0;

    /*! \brief View content
     *
     * This method is supposed open the content in an appropriate app.  Example:
     * if the content is GeoJSON, the content might be opened in Google Earth,
     * or in a mobile mapping application.
     *
     * @param content content text
     *
     * @param mimeType the mimeType of the content
     *
     * @param fileNameTemplate A string of the form "FlightRoute-%1.geojson".
     *
     * @returns Empty string on success, a translated error message otherwise
     */
    Q_INVOKABLE virtual QString viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) = 0;

public slots:
    /*! \brief GUI setup completed
     *
     *  This method is must called as soon as the GUI setup is completed.
     *  On Android, this method will start looking for file import requests
     *  from the OS. On Desktop system, this method does nothing.
     */
    virtual void onGUISetupCompleted() = 0;

    /*! \brief Determine file function and emit openFileRequest()
     *
     * This helper function is called by platform-dependent code whenever the
     * app is asked to open a file.  It will look at the file, determine the
     * file function and emit the signal openFileRequest() as appropriate.
     *
     * There are settings where path points to a temporary file. In these cases,
     * the parameter unmingledFilename can be used to let the system know
     * about the original name of the file. This is useful, e.g., when importing
     * VACs, where the boundary rectangle coordinates are often contained
     * in the file name.
     *
     * @param path File name
     *
     * @param unmigledFilename Unmingled filename
     */
    virtual void processFileOpenRequest(const QString& path, const QString& unmingledFilename);

    /*! \brief Determine file function and emit openFileRequest()
     *
     * Overloaded function for convenience
     *
     * @param path QByteArray containing an UTF8-Encoded strong
     */
    void processFileOpenRequest(const QByteArray& path);

    /*! \brief Process text
     *
     * This helper function is called by platform-dependent code whenever
     * text is passed to the app (e.g. via drag-and-drop or via an
     * Android intent).  It will look at the text, determine the
     * text function and emit signals as appropriate.
     *
     * @param text Text
     */
    void processText(const QString& text);

signals:
    /*! \brief Emitted when platform asks this app to open a file
     *
     * This signal is emitted whenever the platform-dependent code receives
     * information that enroute is requested to open a file.
     *
     * @param fileName Path of the file on the local file system
     *
     * @param info Additional information about the file, as a translated, human-readable string in HTML format.
     *
     * @param fileFunction Function and file type.
     */
    void openFileRequest(QString fileName, QString info, Platform::FileExchange_Abstract::FileFunction fileFunction);

    /*! \brief Emitted when platform asks this app to open a VAC
     *
     * This signal is emitted whenever the platform-dependent code receives
     * information that enroute is requested to open a file containing a VAC.
     *
     * @param vac VAC.
     */
    void openVACRequest(GeoMaps::VAC vac);

    /*! \brief Emitted when platform asks this app to show a waypoint
     *
     * This signal is emitted whenever the platform-dependent code receives
     * information that enroute is requested to show a waypoint.
     *
     * @param waypoint Waypoint to be shown
     */
    void openWaypointRequest(GeoMaps::Waypoint waypoint);

    /*! \brief Emitted when Google Maps URL needs to be resolved
     *
     *  This signal is emitted if the method processText encounters a URL of type
     *  https://maps.app.goo.gl/SOMECODE. The GUI will then open the page
     *  URLResolver.
     */
    void resolveURL(QString url, QString site);

    /*! \brief Emitted when processText was unable to parse a text item */
    void unableToProcessText(QString text);

private:
    Q_DISABLE_COPY_MOVE(FileExchange_Abstract)
};

} // namespace Platform

// Declare meta types
Q_DECLARE_METATYPE(Platform::FileExchange_Abstract)
