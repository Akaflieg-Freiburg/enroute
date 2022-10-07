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

#include "Downloadable_Abstract.h"
#include <chrono>
#include <QLocale>

using namespace std::chrono_literals;


DataManagement::Downloadable_Abstract::Downloadable_Abstract(QObject *parent)
    : QObject(parent)
{
    emitFileContentChanged_delayedTimer.setInterval(2s);
    connect(this, &Downloadable_Abstract::fileContentChanged, &emitFileContentChanged_delayedTimer, qOverload<>(&QTimer::start));
    connect(&emitFileContentChanged_delayedTimer, &QTimer::timeout, this, &Downloadable_Abstract::emitFileContentChanged_delayed);
}



//
// Getter methods
//

auto DataManagement::Downloadable_Abstract::updateSizeString() -> QString
{
    qsizetype size = qMax((qint64)0,updateSize());

    return QLocale::system().formattedDataSize(size, 1, QLocale::DataSizeSIFormat);
}



//
// Setter methods
//


void DataManagement::Downloadable_Abstract::setSection(const QString& sectionName)
{
    if (sectionName == m_section) {
        return;
    }
    m_section = sectionName;
    emit sectionChanged();
}



//
// Private methods
//


void DataManagement::Downloadable_Abstract::emitFileContentChanged_delayed()
{
    if (downloading()) {
        return;
    }
    emitFileContentChanged_delayedTimer.stop();
    emit fileContentChanged_delayed();
}
