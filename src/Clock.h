/***************************************************************************
 *   Copyright (C) 2020 by Stefan Kebekus                                  *
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

#include <QDateTime>
#include <QObject>


/*! \brief This extremely simple class give accss to time and offers a few time-related functions */

class Clock : public QObject {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     * This constructor reads the values of the properties listed below via
     * QSettings. The values are set to NaN if no valid numbers can be found
     * in the settings object.
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Clock(QObject *parent = nullptr);

    // Standard destructor
    ~Clock() override = default;

    /*! Describe time difference in human readable form
     *
     * This method describes the difference between the current time and a given time in human  eadable form.
     *
     * @param pointInTime A point in time that is compared to the current time
     *
     * @returns A localized string such as "just now" (if the pointInTime is rougly equal to the current time), "3 minutes ago" (if the pointInTime is in the past), or "in 1 hour 5 minutes"  (if the pointInTime is in the future)
     */
    Q_INVOKABLE static QString describeTimeDifference(QDateTime pointInTime);

    /*! \brief Current time
     *
     * This property holds the current time in UTC as a string of the form "8:06" or "13:45". The implementation makes some effort to ensure
     * that the notifier signal is emitted just after every minute change.
     */
    Q_PROPERTY(QString time READ time NOTIFY timeChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property time
     */
    QString time() const;

signals:
    /*! \brief Notifier signal */
    void timeChanged();

private:
    // Sets a single shot timer to emit timeChanged just after the full minute
    void setSingleShotTimer();
};
