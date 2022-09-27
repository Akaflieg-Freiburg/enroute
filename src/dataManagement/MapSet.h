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

#include "dataManagement/Downloadable_SingleFile.h"

namespace DataManagement {

class MapSet : public QObject {
    Q_OBJECT

public:
    explicit MapSet(QObject *parent = nullptr);


    //
    // PROPERTIES
    //

    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)

    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

    Q_PROPERTY(bool hasFile READ hasFile NOTIFY hasFileChanged)

    Q_PROPERTY(QString infoText READ infoText NOTIFY infoTextChanged)

    Q_PROPERTY(QString section MEMBER m_section)

    Q_PROPERTY(bool updatable READ updatable NOTIFY updatableChanged)

    Q_PROPERTY(qsizetype updatableSize READ updatableSize NOTIFY updatableSizeChanged)


    //
    // Getter Methods
    //

    [[nodiscard]] auto description() -> QString;

    [[nodiscard]] auto downloading() -> bool;

    [[nodiscard]] auto hasFile() -> bool;

    [[nodiscard]] auto infoText() -> QString;

    [[nodiscard]] auto updatable() -> bool;

    [[nodiscard]] auto updatableSize() -> qsizetype;


    //
    // Methods
    //

    Q_INVOKABLE void add(DataManagement::Downloadable_SingleFile*);
    Q_INVOKABLE void deleteFile();
    Q_INVOKABLE void startFileDownload();
    Q_INVOKABLE void stopFileDownload();
    Q_INVOKABLE void update();

signals:
    /*! \brief Notifier signal for property downloading */
    void descriptionChanged();

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

    /*! \brief Notifier signal for the property updatableSize */
    void updatableSizeChanged();

private:
    QVector<QPointer<DataManagement::Downloadable_SingleFile>> m_maps;
    QString m_section;
};

};
