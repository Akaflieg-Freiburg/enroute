/***************************************************************************
 *   Copyright (C) 2024 by Stefan Kebekus                                  *
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
#include <QFuture>
#include <QGeoRectangle>
#include <QImage>
#include <QQmlEngine>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTimer>

#include "fileFormats/VAC.h"


namespace GeoMaps
{

#warning docu

class VACLibrary : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    /*! \brief Constructor */
    VACLibrary();

    /*! \brief Destructor */
    ~VACLibrary() override = default;

    //
    // Properties
    //

#warning docu
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY dataChanged)

#warning docu
    Q_PROPERTY(QVector<FileFormats::VAC> VACs READ VACs NOTIFY dataChanged)


    //
    // Getter Methods
    //

#warning docu
    [[nodiscard]] bool isEmpty() const { return m_vacs.isEmpty(); }

#warning docu
    [[nodiscard]] QVector<FileFormats::VAC> VACs() const { return m_vacs; }


    //
    // Setter Methods
    //



    //
    // Methods
    //


signals:
    /*! \brief Notifier signal */
    void dataChanged();

private:
    Q_DISABLE_COPY_MOVE(VACLibrary)

    QVector<FileFormats::VAC> m_vacs;
    QString m_vacDirectory {QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/VAC"};

};

} // namespace GeoMaps
