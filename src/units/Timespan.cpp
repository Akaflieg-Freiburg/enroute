/***************************************************************************
 *   Copyright (C) 2019-2025 by Stefan Kebekus                             *
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

#include "units/Timespan.h"

using namespace Qt::Literals::StringLiterals;


QString Units::Timespan::toHoursAndMinutes() const
{
    // Paranoid safety checks
    if (!isFinite())
    {
        return QStringLiteral("-:--");
    }

    auto minutes = qRound(qAbs(toM()));
    auto hours = minutes / 60;
    minutes = minutes % 60;

    QString result;
    if (isNegative())
    {
        result += u"-"_s;
    }
    result += QStringLiteral("%1:%2").arg(hours, 1, 10, QChar(u'0')).arg(minutes, 2, 10, QChar(u'0'));
    return result;
}
