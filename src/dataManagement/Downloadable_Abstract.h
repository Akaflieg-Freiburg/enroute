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

/*! \brief Abstract base class for Downloadable_SingleFile and Downloadable_MultiFile
 *
 *  This is an abstract base class for Downloadable_SingleFile and Downloadable_MultiFile, ensuring that the two classes share a common API.
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

    /*! \brief Indicates whether a download process is currently running */
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

    /*! \brief Indicates if (at least one) local file exists */
    Q_PROPERTY(bool hasFile READ hasFile NOTIFY hasFileChanged)

    /*! \brief Short info text describing the state of the downloadable(s)
     *
     * The text is typically one or more lines of the form
     *
     * - "downloading … 47% complete"
     *
     * - "installed • 203 kB • update available"
     *
     * It might be translated to the local language.
     */
    Q_PROPERTY(QString infoText READ infoText NOTIFY infoTextChanged)

    /*! \brief Headline name for the Downloadable
     *
     * This property is a convenience storing one string along with the
     * Downloadable. The enroute app uses this to store the continent name for a
     * Dowloadable that represents a geographic map.  The GUI then generate
     * section headings in the list of downloadable aviation maps.
     */
    Q_PROPERTY(QString section READ section WRITE setSection NOTIFY sectionChanged)

    /*! \brief Update size
     *
     *  If local files exist and update(s) are available, this property
     *  holds the size of the update. If no updates are available, this property holds 0.
     */
    Q_PROPERTY(qint64 updateSize READ updateSize NOTIFY updateSizeChanged)


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
    [[nodiscard]] virtual auto description() -> QString = 0;

    /*! \brief Getter method for the property with the same name
     *
     * @returns Property downloading
     */
    [[nodiscard]] virtual auto downloading() -> bool = 0;

    /*! \brief Getter method for the property with the same name
     *
     * @returns Property hasFile
     */
    [[nodiscard]] virtual auto hasFile() -> bool = 0;

    /*! \brief Getter method for the property with the same name
     *
     * @returns Property infoText
     */
    [[nodiscard]] virtual auto infoText() -> QString = 0;

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property section
     */
    [[nodiscard]] auto section() const -> QString { return _section; }

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property updateSize
     */
    [[nodiscard]] virtual auto updateSize() -> qint64 = 0;


    //
    // Setter Methods
    //

    /*! \brief Setter function for the property with the same name
     *
     * @param sectionName Property section
     */
    void setSection(const QString& sectionName);


signals:
    /*! \brief Notifier signal */
    void descriptionChanged();

    /*! \brief Notifier signal */
    void downloadingChanged();

    /*! \brief Notifier signal */
    void hasFileChanged();

    /*! \brief Notifier signal */
    void infoTextChanged();

    /*! \brief Notifier signal */
    void sectionChanged();

    /*! \brief Notifier signal */
    void updateSizeChanged();

protected:
    // Property contentType
    ContentType m_contentType {Data};

    // Property section
    QString _section;
private:
    Q_DISABLE_COPY_MOVE(Downloadable_Abstract)
};

};
