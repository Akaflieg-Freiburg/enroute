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

#include "dataManagement/Downloadable_Abstract.h"
#include "dataManagement/Downloadable_SingleFile.h"

namespace DataManagement {

class Downloadable_MultiFile : public Downloadable_Abstract {
    Q_OBJECT

public:
    explicit Downloadable_MultiFile(QObject *parent = nullptr);


    //
    // PROPERTIES
    //

    Q_PROPERTY(bool updatable READ updatable NOTIFY updatableChanged)

    Q_PROPERTY(qsizetype updatableSize READ updatableSize NOTIFY updatableSizeChanged)


    //
    // Getter Methods
    //

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     *  @returns Property description
     */
    [[nodiscard]] auto description() -> QString override;

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     * @returns Property downloading
     */
    [[nodiscard]] auto downloading() -> bool override;

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     * @returns Property hasFile
     */
    [[nodiscard]] auto hasFile() -> bool override;

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     * @returns Property infoText
     */
    [[nodiscard]] auto infoText() -> QString override;

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

    /*! \brief Notifier signal for the property updatable */
    void updatableChanged();

    /*! \brief Notifier signal for the property updatableSize */
    void updatableSizeChanged();

private:
    QVector<QPointer<DataManagement::Downloadable_SingleFile>> m_maps;
};

};
