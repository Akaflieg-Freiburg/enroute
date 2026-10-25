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
#include <QSettings>

#include "FileLibraryModel.h"
#include "GlobalObject.h"
#include "navigation/Aircraft.h"
#include "navigation/FlightRoute.h"
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


    /*! \brief Location of the user manual, see manualLocation() */
    Q_PROPERTY(QString manualLocation READ manualLocation CONSTANT)

    /*! \brief Model listing the entries of the aircraft library, see model() */
    Q_PROPERTY(FileLibraryModel* aircraftModel READ aircraftModel CONSTANT)

    /*! \brief Model listing the entries of the flight route library, see model() */
    Q_PROPERTY(FileLibraryModel* routesModel READ routesModel CONSTANT)

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


    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property aircraftModel
     */
    [[nodiscard]] FileLibraryModel* aircraftModel() const { return m_aircraftModel; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property routesModel
     */
    [[nodiscard]] FileLibraryModel* routesModel() const { return m_routesModel; }

    /*! \brief Model listing the entries of a library
     *
     *  The model follows the library directory and announces changes with
     *  row-granular signals. It is owned by this instance.
     *
     *  @param library The library
     *
     *  @returns Pointer to the model
     */
    [[nodiscard]] FileLibraryModel* model(Librarian::Library library) const;


    /*! \brief Find an aircraft in the library
     *
     *  @param acft Aircraft object
     *
     *  @returns Name of a library entry that equals acft, or an empty string
     *  if there is none
     */
    [[nodiscard]] static QString find(const Navigation::Aircraft& acft);

    /*! \brief Check if a flight route exists in the library
     *
     *  At the moment, only flight routes are supported.
     *
     *  @param route Pointer to the route
     *
     *  @returns True if the object is of type Navigation::FlightRoute and
     *  exists in the library.
     */
    [[nodiscard]] Q_INVOKABLE static bool contains(const QObject* route);

    /*! \brief Name of the directory containing the given
     *
     *  @param library The library that is accessed
     *
     *  @returns Name of the directory, without trailing slash
     */
    [[nodiscard]] Q_INVOKABLE static QString directory(Librarian::Library library) ;

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
    [[nodiscard]] Q_INVOKABLE static Navigation::FlightRoute* get(Librarian::Library library, const QString& baseName);

    /*! \brief Location of the user manual
     *
     *  @returns Directory that holds the HTML version of the manual, as a
     *  path or URL suitable for the platform
     */
    [[nodiscard]] static QString manualLocation();

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

    /*! \brief Saves an aircraft to the library
     *
     *  Writes the aircraft to the library entry, overwriting an existing entry
     *  of the same name, and emits entrySaved().
     *
     *  @param baseName Name of the entry, without path and without extension
     *
     *  @param acft Aircraft
     *
     *  @returns An empty string in case of success, or else a human-readable,
     *  translated error message.
     */
    [[nodiscard]] Q_INVOKABLE static QString saveAircraft(const QString& baseName, const Navigation::Aircraft& acft);

    /*! \brief Name for a new library entry
     *
     *  @param library The library that is accessed
     *
     *  @param baseName Desired name, without path and without extension
     *
     *  @returns baseName if no entry of that name exists, or else
     *  "baseName (n)" with the smallest n that is not taken
     */
    [[nodiscard]] Q_INVOKABLE static QString uniqueName(Librarian::Library library, const QString& baseName);

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

    /*! \brief Checks if a text matches a filter string, in a fuzzy way
     *
     * This method splits the filter into words at whitespace. The text matches
     * if it contains every one of these words. The comparison ignores case and
     * special characters, so that "Zürich" matches "u", "Ü", "ù" and "zurich".
     * An empty filter matches every text.
     *
     * This is the single filter primitive of this app. Lists over item models
     * use it through Ui::NameFilterProxyModel; ad-hoc lists use it from QML,
     * as in
     *
     * model: Array.from(SomeSingleton.items).filter((i) => Librarian.matches(i.name, filterField.filter))
     *
     * @param text Text that is searched
     *
     * @param filter Filter, possibly consisting of several words
     *
     * @returns True if the text matches the filter
     */
    Q_INVOKABLE bool matches(const QString& text, const QString& filter);

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

    /*! \brief Simplifies a string without caching
     *
     * Transforms the string to QString::NormalizationForm_KD and removes every
     * character outside a-z, A-Z and 0-9. This is the stateless core of
     * simplifySpecialChars(). It has no side effects and is safe to call from
     * worker threads, for instance to precompute search keys for item models.
     *
     * @param string Input string
     *
     * @return Simplified string
     */
    [[nodiscard]] static QString simplify(const QString& string);

    /*! \brief Splits a filter into simplified words
     *
     * Splits the filter at whitespace and simplifies every word with
     * simplify(). Words that are empty after simplification are dropped.
     *
     * @param filter Filter text as typed by the user
     *
     * @returns Simplified, non-empty words. Empty if the filter contains no
     * searchable characters.
     */
    [[nodiscard]] static QStringList filterWords(const QString& filter);

    /*! \brief Checks a simplified text against simplified filter words
     *
     * @param simplifiedText Text, already simplified with simplify()
     *
     * @param words Filter words, as returned by filterWords()
     *
     * @returns True if the text contains every word, ignoring case. An empty
     * word list matches every text.
     */
    [[nodiscard]] static bool matchesWords(const QString& simplifiedText, const QStringList& words);

signals:
    /*! \brief Emitted when saveAircraft() has written a library entry
     *
     *  @param library The library that was accessed
     *
     *  @param baseName Name of the entry
     */
    void entrySaved(Librarian::Library library, const QString& baseName);

    /*! \brief Emitted when remove() has removed a library entry
     *
     *  @param library The library that was accessed
     *
     *  @param baseName Name of the entry
     */
    void entryRemoved(Librarian::Library library, const QString& baseName);

    /*! \brief Emitted when rename() has renamed a library entry
     *
     *  @param library The library that was accessed
     *
     *  @param oldName Previous name of the entry
     *
     *  @param newName New name of the entry
     */
    void entryRenamed(Librarian::Library library, const QString& oldName, const QString& newName);

private:
    Q_DISABLE_COPY_MOVE(Librarian)

    FileLibraryModel* m_aircraftModel {nullptr};
    FileLibraryModel* m_routesModel {nullptr};

    // Cache used to speed up the method simplifySpecialChars
    QHash<QString, QString> simplifySpecialChars_cache;

};
