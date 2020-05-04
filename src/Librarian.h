/***************************************************************************
 *   Copyright (C) 2020 by Stefan Kebekus                                  *
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
#include <QRegularExpression>
#include <QSettings>

/*! \brief This simple class helps to manage a library of flight routes */

class Librarian : public QObject {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     * The constructor ensures that the relevant directory for storing the flight route library exists
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Librarian(QObject *parent = nullptr);

    // Standard destructor
    ~Librarian() override = default;

    /*! \brief Lists all flight routes in the library whose name contains the string 'filter'
     *
     * The check for string containment is done in a fuzzy way.
     *
     * @param filter String used to filter the list
     *
     * @returns A filtered QStringList with the base names of flight routes
     *
     * @see permissiveFilter
     */
    Q_INVOKABLE QStringList flightRoutes(const QString &filter=QString());

    /*! \brief Full path of a flight route in the library
     *
     * @param baseName Name of the flight route, without path and without extension
     *
     * @returns Full path of the flight route, with extension
     */
    Q_INVOKABLE QString flightRouteFullPath(const QString &baseName);

    /*! \brief Check if a flight route with the given name exists in the library
     *
     * @param baseName File name, without path and without extension
     *
     * @returns True if the file exists
     */
    Q_INVOKABLE bool flightRouteExists(const QString &baseName);

    /*! \brief Removes a flight route from the library
     *
     * @param baseName File name, without path and without extension
     */
    Q_INVOKABLE void flightRouteRemove(const QString &baseName);

    /*! \brief Filters a QStringList in a fuzzy way
     *
     * This method filters a QStringList. It returns a sublist of those entries whose name approximately contain the filter string.
     * For instance, "Zürich" is supposed to contain "u", "Ü" and "ù"
     *
     * @param in QStringList that is to be filtered
     *
     * @param filter Filter
     *
     * @returns Filteres QStringList
     */
    QStringList permissiveFilter(const QStringList &in, const QString &filter);

    /*! \brief Simplifies string by transforming and removing special characters
     *
     * This method simplifies a unicode string, by transforming it to QString::NormalizationForm_KD and then removing all 'special' character.
     * The results are cached for better performance.
     *
     * @param string Input string
     *
     * @return Simplified string
     */
    QString simplifySpecialChars(const QString &string);

private:
    Q_DISABLE_COPY_MOVE(Librarian)

    QDir flightRouteLibraryDir;

    // Caches used to speed up the method simplifySpecialChars
    QRegularExpression specialChars {"[^a-zA-Z0-9]"};
    QHash<QString, QString> simplifySpecialChars_cache;

};
