/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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

namespace Platform {

/*! \brief Safe window insets for QML
 *
 * Implementations of this class provide QML singletons that describe the
 * safe area screen, there QML objects do not collide with display cutouts or
 * system bars of the operating system.
 */

class SafeInsets_Abstract : public QObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param parent Standard QObject parent pointer
    */
    explicit SafeInsets_Abstract(QObject *parent = nullptr);

    ~SafeInsets_Abstract() override = default;


    //
    // Properties
    //

    /*! \brief Safe inset at bottom of screen, so as to avoid system status bars and display cutouts */
    Q_PROPERTY(double bottom READ bottom NOTIFY bottomChanged)

    /*! \brief Safe inset at left of screen, so as to avoid system status bars and display cutouts */
    Q_PROPERTY(double left READ left NOTIFY leftChanged)

    /*! \brief Safe inset at right of screen, so as to avoid system status bars and display cutouts */
    Q_PROPERTY(double right READ right NOTIFY rightChanged)

    /*! \brief Safe inset at top of screen, so as to avoid system status bars and display cutouts */
    Q_PROPERTY(double top READ top NOTIFY topChanged)

    /*! \brief Height of window, for Android systems
     *
     *  On Android, users can split the screen, in order to show two apps next
     *  to one another (or on top of one another). Neither the ApplicationWindow
     *  QML class nor the QScreen C++ class is aware of this change, so that the
     *  right/bottom part of the app become invisible. The wHeight property here
     *  reflects the usable window height. The property automatically updates
     *  when the split view is switched on/off, or when the relative size of the
     *  windows changes.
     *
     *  On systems other than Android, this property contains NaN.
     */
    Q_PROPERTY(double wHeight READ wHeight NOTIFY wHeightChanged)

    /*! \brief Width of window, for Android systems
     *
     *  @see wHeight
     */
    Q_PROPERTY(double wWidth READ wWidth NOTIFY wWidthChanged)


    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property bottom
     */
    double bottom() const {return m_bottom;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property left
     */
    double left() const {return m_left;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property right
     */
    double right() const {return m_right;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property top
     */
    double top() const {return m_top;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property wHeight
     */
    double wHeight() const {return m_wHeight;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property wWidth
     */
    double wWidth() const {return m_wWidth;}

signals:
    /*! \brief Notifier signal */
    void bottomChanged();

    /*! \brief Notifier signal */
    void leftChanged();

    /*! \brief Notifier signal */
    void rightChanged();

    /*! \brief Notifier signal */
    void topChanged();

    /*! \brief Notifier signal */
    void wHeightChanged();

    /*! \brief Notifier signal */
    void wWidthChanged();

protected:
    // Member variables
    double m_bottom {0.0};
    double m_left {0.0};
    double m_right {0.0};
    double m_top {0.0};
    double m_wHeight {qQNaN()};
    double m_wWidth {qQNaN()};

private:
    Q_DISABLE_COPY_MOVE(SafeInsets_Abstract)

};

} // namespace Platform
