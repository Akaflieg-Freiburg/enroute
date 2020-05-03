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

#include <QRegularExpression>
#include <QSettings>

/*! \brief This simple class helps to manage a library of flight routes */

class Library : public QObject {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Library(QObject *parent = nullptr);

    // Standard destructor
    ~Library() override = default;

#warning docu
    Q_INVOKABLE QStringList flightRoutes(const QString &filter=QString());

    Q_INVOKABLE bool flightRouteExists(const QString &baseName);

    QStringList permissiveFilter(const QStringList &in, const QString &filter);

    QString simplifySpecialChars(const QString &string);

private:
    Q_DISABLE_COPY_MOVE(Library)

    // Caches used to speed up the method simplifySpecialChars
    QRegularExpression specialChars {"[^a-zA-Z0-9]"};
    QHash<QString, QString> simplifySpecialChars_cache;

};
