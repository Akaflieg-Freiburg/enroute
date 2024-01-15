/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

#include "FileExchange_Abstract.h"

namespace Platform {

/*! \brief Template implementation of FileExchange */

class FileExchange : public FileExchange_Abstract
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param parent Standard QObject parent pointer
    */
    explicit FileExchange(QObject *parent = nullptr);

    ~FileExchange() override = default;


    //
    // Methods
    //

    /*! \brief Implements pure virtual method from FileExchange_Abstract */
    void importContent() override;

    /*! \brief Implements pure virtual method from FileExchange_Abstract
     *
     *  @param content see documentation for FileExchange_Abstract
     *
     *  @param mimeType see documentation for FileExchange_Abstract
     *
     *  @param fileNameTemplate see documentation for FileExchange_Abstract
     *
     *  @returns see documentation for FileExchange_Abstract
     */
    QString shareContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) override;

    /*! \brief Implements pure virtual method from FileExchange_Abstract
     *
     *  @param content see documentation for FileExchange_Abstract
     *
     *  @param mimeType see documentation for FileExchange_Abstract
     *
     *  @param fileNameTemplate see documentation for FileExchange_Abstract
     *
     *  @returns see documentation for FileExchange_Abstract
     */
    QString viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) override;


protected:
    /*! \brief Implements virtual method from GlobalObject */
    virtual void deferredInitialization() override;

private:
    Q_DISABLE_COPY_MOVE(FileExchange)
};

} // namespace Platform
