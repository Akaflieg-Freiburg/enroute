/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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

/*! \brief Wrapper around size_t, to make it available to QML
 *
 *  This is a trivial C++ and QML value type, with a single member of type
 *  size_t. Conversion methods guarantee that this class can be used in C++ as a
 *  drop-in replacement for size_t. In QML, the class can be used to work to
 *  size_t, which is otherwise not available.
 */

namespace Units {

    /*! \brief Convenience class for size_t
     */
    class ByteSize {
        Q_GADGET
        QML_VALUE_TYPE(byteSize)

    public:
        ByteSize() = default;
        ByteSize(const ByteSize&) = default;

        /*! \brief Conversion from size_t to Units::ByteSize
         *
         * @param val Value
         */
        ByteSize(size_t val) : value(val) {}

        /*! \brief Check if zero */
        Q_INVOKABLE bool isNull() { return value==0; }

        /*! \brief Conversion from Units::ByteSize to size_t
         *
         * @returns Value
         */
        operator size_t() const {return value;}

    private:
        size_t value {0};
    };
} // namespace Units

// Declare meta types
Q_DECLARE_METATYPE(Units::ByteSize)
