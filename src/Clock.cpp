/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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

#include "Clock.h"

#include <QDebug>
#include <QGuiApplication>
#include <QTimer>


Clock::Clock(QObject *parent) : QObject(parent)
{
    // We need to update the time regularly. I do not use a simple timer here that emits "timeChanged" once per minute, because I
    // want the signal to be emitted right after the full minute. So, I use a timer that once a minute set a single-shot time
    // that is set to fire up 500ms after the full minute. This design will also work reliably if "timer" get out of synce,
    // for instance because the app was sleeping for a while.
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Clock::setSingleShotTimer);
    timer->setInterval(60*1000);
    timer->start();

    // Start the single shot timer once manually
    setSingleShotTimer();

    // There are a few other events where we want to update the clock
    connect(qGuiApp, &QGuiApplication::applicationStateChanged, this, &Clock::timeChanged);
}


QString Clock::describeTimeDifference(QDateTime pointInTime)
{
    auto minutes = qRound(QDateTime::currentDateTime().secsTo(pointInTime)/60.0);

    bool past = minutes < 0;
    minutes = qAbs(minutes);

    if (minutes == 0)
        return tr("just now");

    auto hours = minutes/60;
    minutes = minutes%60;

    QString result = "";
    if ((hours != 0) && (minutes == 0))
        result = tr("%1h").arg(hours);
    else if ((hours == 0) && (minutes != 0))
        result = tr("%1min").arg(minutes);
    else
        result = tr("%1h and %2min").arg(hours).arg(minutes);

    if (past)
        result = tr("%1 ago").arg(result);
    else
        result = tr("in %1").arg(result);

    return result.simplified();
}


QString Clock::describePointInTime(QDateTime pointInTime, QGeoCoordinate position)
{
    pointInTime = pointInTime.toUTC();

    if (position.isValid()) {
        auto lastMidnight = QDateTime::currentDateTimeUtc();
        lastMidnight.setTime( QTime::fromMSecsSinceStartOfDay(4.0*position.longitude()*60.0*1000.0) );
        if (lastMidnight > QDateTime::currentDateTime())
            lastMidnight = lastMidnight.addDays(-1);

        if ((pointInTime < lastMidnight) && (pointInTime > lastMidnight.addDays(-1)))
            return tr("yesterday %1").arg(pointInTime.toString("H:mm"));
        if ((pointInTime > lastMidnight) && (pointInTime < lastMidnight.addDays(1)))
            return pointInTime.toString("H:mm");
        if ((pointInTime > lastMidnight.addDays(1)) && (pointInTime < lastMidnight.addDays(2)))
            return tr("tomorrow %1").arg(pointInTime.toString("H:mm"));
    }

    return pointInTime.toString("d. MMM, H:mm");
}


void Clock::setSingleShotTimer()
{
    QTime current = QTime::currentTime();
    int msecsToNextMinute = 60*1000 - (current.msecsSinceStartOfDay() % (60*1000));
    QTimer::singleShot(msecsToNextMinute+500, this, &Clock::timeChanged);
}


QString Clock::timeAsUTCString() const
{
    return QDateTime::currentDateTimeUtc().toString("H:mm");
}
