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

#include <QObject>


namespace DataManagement {

/*! \brief Abstract base class Downloadable_SingleFile and
 *  Downloadable_MultiFile
 *
 *  This is an abstract base class Downloadable_SingleFile and
 *  Downloadable_MultiFile, ensuring that the two classes share a common API.
 */

class Downloadable_Abstract : public QObject {
    Q_OBJECT

public:
    /*! \brief Type of content managed by this instance */
    enum ContentType {
        AviationMap,    /*!< \brief Aviation Map */
        BaseMapVector,  /*!< \brief Base Map, in vector format */
        BaseMapRaster,  /*!< \brief Base Map, in raster format */
        Data,           /*!< \brief Data */
        MapSet,         /*!< \brief Set of maps */
        TerrainMap      /*!< \brief Terrain Map */
    };
    Q_ENUM(ContentType)

    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer.
     */
    explicit Downloadable_Abstract(QObject *parent = nullptr);



    //
    // PROPERTIES
    //

    /*! \brief Most probable content of file(s) managed by this object */
    Q_PROPERTY(DataManagement::Downloadable_Abstract::ContentType contentType READ contentType CONSTANT)

    /*! \brief Describe installed file(s)
     *
     * This property contains a localized description of the locally installed file(s), localized and in HTML format.
     * If no description is available, then the property contains an empty string.
     */
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)


    //
    // Getter Methods
    //

    /*! \brief Getter method for the property with the same name
     *
     *  @returns Property contentType
     */
    [[nodiscard]] auto contentType() const -> DataManagement::Downloadable_Abstract::ContentType {return m_contentType;}

    /*! \brief Getter method for the property with the same name
     *
     *  @returns Property description
     */
    [[nodiscard]] auto description() const -> QString {return m_description;}

signals:
    /*! \brief Notifier signal */
    void descriptionChanged(QString description);

protected:
    // Setter method
    void setDescription(const QString& description);

    // Property contentType
    ContentType m_contentType {Data};

private:
    Q_DISABLE_COPY_MOVE(Downloadable_Abstract)

    // Property description
    QString m_description;
};

};
