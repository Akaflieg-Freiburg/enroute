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



    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property bottom
     */
    double bottom() const {return _bottom;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property left
     */
    double left() const {return _left;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property right
     */
    double right() const {return _right;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property top
     */
    double top() const {return _top;}


signals:
    /*! \brief Notifier signal */
    void bottomChanged();

    /*! \brief Notifier signal */
    void leftChanged();

    /*! \brief Notifier signal */
    void rightChanged();

    /*! \brief Notifier signal */
    void topChanged();

protected:
    // Member variables
    double _bottom {0.0};
    double _left {0.0};
    double _right {0.0};
    double _top {0.0};

private:
    Q_DISABLE_COPY_MOVE(SafeInsets_Abstract)

};

} // namespace Platform
