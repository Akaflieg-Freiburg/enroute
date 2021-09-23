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

#include <QCache>
#include <QObject>

#include "dataManagement/Downloadable.h"

namespace Traffic {


/*! \brief Flarmnet database
 *
 *  This simple class provides access to a Flarmnet database, which is in essence
 *  a glorified QHash<QString, QString>, where keys are Flarm IDs and values
 *  are aircraft registration strings.
 */
class FlarmnetDB : public QObject {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     *  This default constructor will read the database from a file into memory.
     *
     *  @param parent The standard QObject parent pointer
     */
    FlarmnetDB(QObject* parent);

    ~FlarmnetDB() = default;

    //
    // Methods
    //

    /*! \brief Find password for a given key
     *
     *  @param key FlarmID to look up
     *
     *  @returns Aircraft registration, or an empty string if the database does not contain the key
     */
    Q_INVOKABLE QString getRegistration(const QString& key);

private slots:
    // The title says everything
    void clearCache();

    // The title says everything
    void deferredInitialization();

    // The title says everything
    void findFlarmnetDBDownloadable();

private:
    QString getRegistrationFromFile(const QString& key);

    QPointer<DataManagement::Downloadable> flarmnetDBDownloadable;

    QCache<QString, QString> m_cache {};
};

}
