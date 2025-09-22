/***************************************************************************
 *   Copyright (C) 2020-2023 by Stefan Kebekus                             *
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

#include <QDir>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QSettings>

#include "GlobalObject.h"
#include "navigation/Aircraft.h"
#include "units/ByteSize.h"


/*! \brief Manage libraries and text assets
 *
 *  This class manages libraries of aircrafts and routes, as well as text assets, and exposes these objects to QML.
 *
 */

class Librarian : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    /*! \brief Default constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Librarian(QObject *parent = nullptr);

    // No default constructor, important for QML singleton
    explicit Librarian() = delete;

    // Standard destructor
    ~Librarian() override = default;


    // factory function for QML singleton
    static Librarian* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::librarian();
    }

    /*! \brief Type of library */
    enum Library : quint8 {
        /*! \brief Aircraft library */
        Aircraft,

        /*! \brief Flight route library */
        Routes
    };
    Q_ENUM(Library)


    /*! \brief Check if an object exists in the library
     *
     *  @param acft Aircraft object
     *
     *  @returns True if the aircraft exists in the library.
     */
    [[nodiscard]] Q_INVOKABLE static bool contains(const Navigation::Aircraft& acft);

    /*! \brief Check if an object exists in the library
     *
     *  At the moment, only flight routes are supported.
     *
     *  @param object Pointer to an object
     *
     *  @returns True if the object is of type Navigation::FlightRoute and exists in the library.
     */
    [[nodiscard]] Q_INVOKABLE static bool contains(QObject* route);

    /*! \brief Name of the directory containing the given
     *
     *  @param library The library that is accessed
     *
     *  @returns Name of the directory, without trailing slash
     */
    [[nodiscard]] Q_INVOKABLE static QString directory(Librarian::Library library) ;

    /*! \brief Lists all entries in the library whose name contains the string 'filter'
     *
     * The check for string containment is done in a fuzzy way.
     *
     * @param library The library that is to be searched
     *
     * @param filter String used to filter the list
     *
     * @returns A filtered QStringList with the base names of flight routes
     *
     * @see permissiveFilter
     */
    Q_INVOKABLE QStringList entries(Librarian::Library library, const QString& filter=QString());

    /*! \brief Check if an entry with the given name exists in the library
     *
     *  @param library The library that is accessed
     *
     *  @param baseName File name, without path and without extension
     *
     *  @returns True if the file exists
     */
    [[nodiscard]] Q_INVOKABLE static bool exists(Librarian::Library library, const QString& baseName) ;

    /*! \brief Full path of a library entry
     *
     *  @param library The library that is accessed
     *
     *  @param baseName Name of the entry, without path and without
     *  extension
     *
     *  @returns Full path of the entry, with extension
     */
    [[nodiscard]] Q_INVOKABLE static QString fullPath(Librarian::Library library, const QString& baseName) ;

    /*! \brief Constructs an object from library entry
     *
     * This method constructs an object from a library entry.
     *
     * If the entry is a route, the route is construted with aircraft and wind
     * set to nullptr, so that no wind computations are possible. It is,
     * however, possible to export the flight route (for instance to GeoJSON or
     * GPX format).
     *
     * Ownership is transferred to the caller, so it is up to the caller to
     * delete the flight route once it is no longer used. Note that QML does
     * that automatically.
     *
     * @param library The library that is accessed
     *
     * @param baseName File name, without path and without extension
     *
     * @returns Pointer to the object, or a nullptr in case of error.
     */
    [[nodiscard]] Q_INVOKABLE static QObject* get(Librarian::Library library, const QString& baseName) ;

    /*! \brief Exposes string stored in QRessource to QML
     *
     * This method reads a string from a file stored in the QRessource
     * system. The method expects that the file contains a string in UTF8
     * encoding.  There are a few special strings that are not read from
     * QRessource, but are stored internally and that will be translated. These
     * are the following.
     *
     * - ":text/whatsnew.html" A text that describes new features in the current
     *   program version
     *
     * @param name Name of the file in the QRessource, such as
     * ":text/bugReport.html"
     *
     * @returns File content as a QString
     */
    Q_INVOKABLE static QString getStringFromRessource(const QString& name);

    /*! \brief Exposes the hash of string stored in QRessource to QML
     *
     * This method reads a string from a file stored in the QRessource
     * system. The method expects that the file contains a string in UTF8
     * encoding.
     *
     * @param name Name of the file in the QRessource, such as ":text/bugReport.html"
     *
     * @returns Hash of file content
     */
    Q_INVOKABLE static Units::ByteSize getStringHashFromRessource(const QString& name);

    /*! \brief Import a file into a library
     *
     * This method imports a file into a library.
     *
     * @param library The library to import the file. At the moment, only routs is supported.
     *
     * @param fileName Full file name
     *
     * @returns An empty string in case of success, or else a human-readable, translated error message.
     */
    [[nodiscard]] Q_INVOKABLE static QString import(Librarian::Library library, const QString& fileName) ;

    /*! \brief Removes an entry from a library
     *
     *  @param library The library that is accessed
     *
     *  @param baseName File name, without path and without extension
     */
    Q_INVOKABLE static void remove(Librarian::Library library, const QString& baseName) ;

    /*! \brief Renames an entry in a library
     *
     *  @param library The library that is accessed
     *
     * @param oldName Name of the file that is to be renamed, without path and
     * without extension
     *
     * @param newName New file name, without path and without extension. A file
     * with that name must not exist in the library
     */
    Q_INVOKABLE static void rename(Librarian::Library library, const QString& oldName, const QString& newName);

    /*! \brief Filters a QStringList in a fuzzy way
     *
     * This helper method filters a QStringList. It returns a sublist of those
     * entries whose name approximately contain the filter string.  For
     * instance, "Zürich" is supposed to contain "u", "Ü" and "ù"
     *
     * @param input QStringList that is to be filtered
     *
     * @param filter Filter
     *
     * @returns Filteres QStringList
     */
    QStringList permissiveFilter(const QStringList &input, const QString &filter);

    /*! \brief Simplifies string by transforming and removing special characters
     *
     * This helper method simplifies a unicode string, by transforming it to
     * QString::NormalizationForm_KD and then removing all 'special' character.
     * The results are cached for better performance.
     *
     * @param string Input string
     *
     * @return Simplified string
     */
    QString simplifySpecialChars(const QString& string);

private:
    Q_DISABLE_COPY_MOVE(Librarian)

    // Caches used to speed up the method simplifySpecialChars
    QRegularExpression specialChars {QStringLiteral("[^a-zA-Z0-9]")};
    QHash<QString, QString> simplifySpecialChars_cache;

};
