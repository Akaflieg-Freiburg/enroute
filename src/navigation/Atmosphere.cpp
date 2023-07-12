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

#include <QQmlEngine>
#include "navigation/Atmosphere.h"


//
// Constructors and destructors
//

Navigation::Atmosphere::Atmosphere(QObject *parent) : GlobalObject(parent)
{
#if defined(Q_OS_ANDROID) or defined(Q_OS_IOS)
    m_pressureSensor.setActive(true);
    m_temperatureSensor.setActive(true);

    auto* timer = new QTimer(this);
    timer->setInterval(1000);
    timer->setSingleShot(false);
    timer->start();
    qWarning() << "A";
    connect(timer, &QTimer::timeout, this, [this]()
            {
        qWarning() << "AA";
        auto* reading = m_pressureSensor.reading();
        if (reading != nullptr)
        {
            qDebug() << "Pressure" << reading->pressure();
        }


        auto* treading = m_temperatureSensor.reading();
        if (treading != nullptr)
        {
            qDebug() << "Temperature" << treading->temperature();
        }

    });
#endif

}


void Navigation::Atmosphere::deferredInitialization()
{
}



//
// Slots
//


void Navigation::Atmosphere::updateSensorReadings()
{
#warning not implemented
}
