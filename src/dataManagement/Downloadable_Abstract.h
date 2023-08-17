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

#include <QGeoRectangle>
#include <QQmlEngine>
#include <QTimer>

#include "units/ByteSize.h"


namespace DataManagement {

/*! \brief Abstract base class for Downloadable_SingleFile and
 *  Downloadable_MultiFile
 *
 *  This is an abstract base class for Downloadable_SingleFile and
 *  Downloadable_MultiFile, ensuring that the two classes share a common API.
 *
 *  @note Subclasses need to set the protected member m_contentType in their
 *  constructors.
 */

class Downloadable_Abstract : public QObject {
    Q_OBJECT
    QML_ELEMENT

public:
    /*! \brief Type of content managed by this instance */
    enum ContentType {
        ApproachChart,  /*!< \brief Approach Chart */
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
    explicit Downloadable_Abstract(QObject* parent = nullptr);



    //
    // PROPERTIES
    //

    /*! \brief Most probable content of file(s) managed by this object */
    Q_PROPERTY(QGeoRectangle boundingBox READ boundingBox CONSTANT)

    /*! \brief Most probable content of file(s) managed by this object */
    Q_PROPERTY(DataManagement::Downloadable_Abstract::ContentType contentType READ contentType CONSTANT)

    /*! \brief Describe installed file(s)
     *
     * This property contains a description of the locally installed
     * file(s), localized and in HTML format. If no description is available,
     * then the property contains an empty string.
     */
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)

    /*! \brief Indicates whether a download process is currently running */
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

    /*! \brief Names of all files that have been downloaded by any of the
     *  Downloadble objects in this group (and their children)
     */
    Q_PROPERTY(QStringList files READ files NOTIFY filesChanged)

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

    /*! \brief Size of the remote file
     *
     * This property holds the size of the remote files.  If the sizes are
     * not known, the property holds the number -1.
     */
    Q_PROPERTY(qint64 remoteFileSize READ remoteFileSize NOTIFY remoteFileSizeChanged)

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
     *  If local files exist and update(s) are available, this property holds
     *  the size of the update. If no updates are available, this property holds
     *  0.
     */
    Q_PROPERTY(Units::ByteSize updateSize READ updateSize NOTIFY updateSizeChanged)

    /*! \brief Update size as a localized string */
    Q_PROPERTY(QString updateSizeString READ updateSizeString NOTIFY updateSizeChanged)


    //
    // Getter Methods
    //

    /*! \brief Getter method for the property with the same name
     *
     *  @returns Property boundingBox
     */
    [[nodiscard]] auto boundingBox() const -> QGeoRectangle {return m_boundingBox;}

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
     * @returns Property files
     */
    [[nodiscard]] virtual auto files() -> QStringList = 0;

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
     * @returns Property remoteFileSize
     */
    [[nodiscard]] virtual auto remoteFileSize() -> qint64 = 0;

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property section
     */
    [[nodiscard]] auto section() const -> QString { return m_section; }

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property updateSize
     */
    [[nodiscard]] virtual auto updateSize() -> Units::ByteSize = 0;

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property updateSizeString
     */
    [[nodiscard]] virtual auto updateSizeString() -> QString;



    //
    // Setter Methods
    //

    /*! \brief Setter function for the property with the same name
     *
     * @param sectionName Property section
     */
    void setSection(const QString& sectionName);



    //
    // Methods
    //

    /*! \brief Deletes all local file(s) */
    Q_INVOKABLE virtual void deleteFiles() = 0;

    /*! \brief Kill pending emission of signal fileContentChanged_delayed */
    Q_INVOKABLE void killFileContentChanged_delayed()
    {
        emitFileContentChanged_delayedTimer.stop();
    }

    /*! \brief Initiate download(s) */
    Q_INVOKABLE virtual void startDownload() = 0;

    /*! \brief Stops download process(es) */
    Q_INVOKABLE virtual void stopDownload() = 0;

    /*! \brief Starts download(s) if updatable */
    Q_INVOKABLE virtual void update() = 0;

signals:
    /*! \brief Notifier signal */
    void descriptionChanged();

    /*! \brief Notifier signal */
    void downloadingChanged();

    /*! \brief Download error
     *
     * This signal is emitted if the download process fails for whatever reason.
     * Once the signal is emitted, the download process is deleted and no
     * further actions will take place. The local file will not be touched.
     *
     * @param objectName Name of this QObject, as obtained by the method
     * objectName()
     *
     * @param message A brief error message of the form "the requested resource
     * is no longer available at the server", possibly translated.
     */
    void error(QString objectName, QString message);

    /*! \brief Indicates that the content of a local file (or several local files) has changed */
    void fileContentChanged();

    /*! \brief Emitted some time after the content of one of the local files changes.
     *
     *  This signal is in principle identical to fileContentChanged(), but
     *  is emitted with a delay of two seconds. In addition it waits until
     *  there are no running download processes anymore.
     */
    void fileContentChanged_delayed();

    /*! \brief Notifier signal */
    void filesChanged();

    /*! \brief Notifier signal */
    void hasFileChanged();

    /*! \brief Notifier signal */
    void infoTextChanged();

    /*! \brief Notifier signal */
    void remoteFileSizeChanged();

    /*! \brief Notifier signal */
    void sectionChanged();

    /*! \brief Notifier signal */
    void updateSizeChanged();

protected:
    // Property contentType
    QGeoRectangle m_boundingBox;

    // Property contentType
    ContentType m_contentType {Data};

    // Property section
    QString m_section;
private:
    Q_DISABLE_COPY_MOVE(Downloadable_Abstract)

    // Provisions to provide the signal localFileContentChanged_delayed
    void emitFileContentChanged_delayed();
    QTimer emitFileContentChanged_delayedTimer;
};

} // namespace DataManagement
