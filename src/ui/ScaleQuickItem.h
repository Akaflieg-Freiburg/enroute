/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

#include <QQmlEngine>
#include <QtQuick/QQuickPaintedItem>

namespace Ui {

/*! \brief QML Class implementing a scale for a Map
 *
 *  This class implements a QML item that draws a scale, to be added to a map.  To
 *  work, the property pixelPer10km needs to be set, according to the scale of the
 *  map.  To use this class, export it to QML, add it to your view and make sure
 *  that the property pixelPer10km get set accordingly.
 *
 *  The methods of this class are re-entrant, but not thread safe.
 */

class ScaleQuickItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Scale)

public:
    /*! \brief Standard constructor
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit ScaleQuickItem(QQuickItem *parent = nullptr);

    // Default destructor
    ~ScaleQuickItem() override = default;

    /*! \brief Foreground color */
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged REQUIRED)

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property pixelPer10km
     */
    [[nodiscard]] QColor color() const {return m_color;}

    /*! \brief Setter function for the property with the same name
     *
     *  @param _color Property color
     */
    void setColor(QColor _color);

    /*! \brief Number of pixel that represent a distance of 10km on the map */
    Q_PROPERTY(qreal pixelPer10km READ pixelPer10km WRITE setPixelPer10km NOTIFY pixelPer10kmChanged)

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property pixelPer10km
     */
    [[nodiscard]] auto pixelPer10km() const -> qreal {return m_pixelPer10km;}

    /*! \brief Setter function for the property with the same name
     *
     *  @param _pxp10k  Property pixelPer10km
     */
    void setPixelPer10km(qreal _pxp10k);

    /*! \brief Re-implemented from QQuickPaintedItem to implement painting
     *
     *  @param painter Pointer to the QPainter used for painting
     */
    void paint(QPainter* painter) override;

    /*! \brief Determines whether the scale should be drawn vertically or horizontally
     *
     *  This property defaults to 'false'.
     */
    Q_PROPERTY(bool vertical READ vertical WRITE setVertical NOTIFY verticalChanged)

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property vertical
     */
    [[nodiscard]] auto vertical() const -> bool {return m_vertical;}

    /*! \brief Setter function for the property with the same name
     *
     *  @param newVertical Property vertical
     */
    void setVertical(bool newVertical);

signals:
    /*! \brief Notification signal for property with the same name */
    void colorChanged();

    /*! \brief Notification signal for property with the same name */
    void pixelPer10kmChanged();

    /*! \brief Notification signal for property with the same name */
    void verticalChanged();

private:
    Q_DISABLE_COPY_MOVE(ScaleQuickItem)

    qreal m_pixelPer10km {0.0};
    bool m_vertical {false};
    QColor m_color {Qt::black};
};

} // namespace Ui
