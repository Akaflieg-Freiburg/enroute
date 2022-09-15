/***************************************************************************
 *   Copyright (C) 2022 by Stefan Kebekus                                  *
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

#include "dataManagement/Downloadable.h"

namespace DataManagement {

class MapSet : public QObject {
    Q_OBJECT

public:
    explicit MapSet(QObject *parent = nullptr);

    /*! \brief Indicates if the file the has been downloaded is known to be updatable
     *
     * This property is true if all of the following conditions are met.
     *
     * - No download is in progress
     *
     * - The file has been downloaded
     *
     * - The modification date of the file on the remote server
     *   is newer than the modification date of the local file.
     *
     * @warning The notification signal is not emitted when another process
     * touches the local file.
     */
    Q_PROPERTY(bool updatable READ updatable NOTIFY updatableChanged)

    /*! \brief Indicates whether a download process is currently running
     *
     * This property indicates whether a download process is currently running
     *
     * @see startFileDownload(), stopFileDownload()
     */
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property downloading
     */
    [[nodiscard]] auto downloading() const -> bool;

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property updatable
     */
    [[nodiscard]] auto updatable() const -> bool;

    QPointer<DataManagement::Downloadable> baseMap;
    QPointer<DataManagement::Downloadable> terrainMap;

signals:

    /*! \brief Notifier signal for property downloading */
    void downloadingChanged();

    /*! \brief Notifier signal for the property updatable */
    void updatableChanged();


};

};
