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


/*! \brief Group of closely related downloadable items
 *
 *  This class implements a group of downloadable item, that is of
 *  Downloadable_SingleFile objects.
 *
 *  The members of the group are closely related: the Downloadable_MultiFile is
 *  considered updatable if one of its members are updatable, or if one (but not
 *  all) of its members has a local file. Calling update() will then update all
 *  updatable members, and download all missing files.
 */


class Downloadable_MultiFile : public Downloadable_Abstract {
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer.
     */
    explicit Downloadable_MultiFile(QObject *parent = nullptr);


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

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     * @returns Property updateSize
     */
    [[nodiscard]] auto updateSize() -> qint64 override;


    //
    // Methods
    //

    /*! \brief Add a Downloadable_SingleFile to this Downloadable_MultiFile */
    Q_INVOKABLE void add(DataManagement::Downloadable_SingleFile*);

    /*! \brief Implementation of pure virtual method from Downloadable_Abstract */
    Q_INVOKABLE void deleteFiles() override;

    /*! \brief Implementation of pure virtual method from Downloadable_Abstract */
    Q_INVOKABLE void startDownload() override;

    /*! \brief Implementation of pure virtual method from Downloadable_Abstract */
    Q_INVOKABLE void stopDownload() override;

    /*! \brief Implementation of pure virtual method from Downloadable_Abstract */
    Q_INVOKABLE void update() override;

private:
    QVector<QPointer<DataManagement::Downloadable_SingleFile>> m_maps;
};

};
