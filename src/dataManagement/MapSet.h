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
    explicit MapSet(DataManagement::Downloadable* baseMap, DataManagement::Downloadable* terrainMap, QObject *parent = nullptr);


    //
    // PROPERTIES
    //

    Q_PROPERTY(QString section MEMBER m_section)

    Q_PROPERTY(QString infoText READ infoText NOTIFY infoTextChanged)

    Q_PROPERTY(bool updatable READ updatable NOTIFY updatableChanged)

    Q_PROPERTY(bool hasFile READ hasFile NOTIFY hasFileChanged)

    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)


    //
    // Getter Methods
    //

    [[nodiscard]] auto downloading() const -> bool;

    [[nodiscard]] auto hasFile() const -> bool;

    [[nodiscard]] auto infoText() const -> QString;

    [[nodiscard]] auto updatable() const -> bool;


    //
    // Methods
    //

    Q_INVOKABLE void deleteFile();
    Q_INVOKABLE void startFileDownload();

private:
    QPointer<DataManagement::Downloadable> m_baseMap;
    QPointer<DataManagement::Downloadable> m_terrainMap;

    QString m_section;

signals:
    /*! \brief Notifier signal for property downloading */
    void downloadingChanged();

    /*! \brief Download error
     *
     * This signal is emitted if the download process fails for whatever
     * reason. Once the signal is emitted, the download process is deleted and
     * no further actions will take place. The local file will not be touched.
     *
     * @param objectName Name of this QObject, as obtained by the method
     * objectName()
     *
     * @param message A brief error message of the form "the requested resource
     * is no longer available at the server", possibly translated.
     */
    void error(QString objectName, QString message);

    /*! \brief Notifier signal for property hasFile */
    void hasFileChanged();

    /*! \brief Notifier signal for property downloading */
    void infoTextChanged();

    /*! \brief Notifier signal for the property updatable */
    void updatableChanged();
};

};
