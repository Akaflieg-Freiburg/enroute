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
#include <QGeoCoordinate>
#include <QObject>


/*! \brief This extremely simple class give accss to time and offers a few time-related functions
 *
 *
 * There exists one static instance of this class, which can be accessed via the
 * method globalInstance().  No other instance of this class should be used.
 */

class Clock : public QObject {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Clock(QObject *parent = nullptr);

    // Standard destructor
    ~Clock() override = default;

    /*! \brief Current date
     *
     * This property holds the current date for the local time zone. The implementation makes some effort to ensure
     * that the notifier signal is emitted just after midnight in the local time zone.
     */
    Q_PROPERTY(QDate date READ date NOTIFY dateChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property date
     */
    QDate date() const
    {
        return QDateTime::currentDateTime().date();
    }

    /*! Describe time difference in human readable form
     *
     * This method describes the difference between the current time and a given time in human readable form.
     *
     * @param pointInTime A point in time that is compared to the current time
     *
     * @returns A localized string such as "just now" (if the pointInTime is rougly equal to the current time), "3 minutes ago" (if the pointInTime is in the past), or "in 1 hour 5 minutes"  (if the pointInTime is in the future)
     */
    Q_INVOKABLE static QString describeTimeDifference(const QDateTime& pointInTime);

    /*! Describe a point in time in human-readable form
     *
     * This method describes a point in time in human readable form. The method returns a localized string of the form
     * "12. Sept., 17:51", "tomorrow 17:51", "yesterday 17:51" or "17:51". The words "tomorrow" and "yesterday" refer midnight
     * at the current position, the numbers "17:51" refer to time in UTC.
     *
     * \param pointInTime The point in time that is to be described.
     *
     * \return String with the description
     */
    Q_INVOKABLE static QString describePointInTime(QDateTime pointInTime);

    /*! \brief Pointer to static instance
     *
     * This method returns a pointer to a static instance of this class. In rare
     * situations, during shutdown of the app, a nullptr might be returned.
     *
     * @returns A pointer to a static instance of this class
     */
    static Clock *globalInstance();

    /*! \brief Current time in UTC as a string
     *
     * This property holds the current time in UTC as a string of the form "8:06" or "13:45". The implementation makes some effort to ensure
     * that the notifier signal is emitted just after every minute change.
     */
    Q_PROPERTY(QString timeAsUTCString READ timeAsUTCString NOTIFY timeChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property time
     */
    static QString timeAsUTCString() ;

    /*! \brief Current time
     *
     * This property holds the current time. The implementation makes some effort to ensure
     * that the notifier signal is emitted just after every minute change.
     */
    Q_PROPERTY(QDateTime time READ time NOTIFY timeChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property time
     */
    QDateTime time() const
    {
        return QDateTime::currentDateTime();
    }

signals:
    /*! \brief Notifier signal */
    void dateChanged();

    /*! \brief Notifier signal */
    void timeChanged();

private:
    // Sets a single shot timer to emit timeChanged just after the full minute
    void setSingleShotTimer();
};
