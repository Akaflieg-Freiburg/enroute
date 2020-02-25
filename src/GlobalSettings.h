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

#ifndef GlobalSettings_H
#define GlobalSettings_H

#include <QObject>
#include <QSettings>


/*! \brief Global Settings Manager
  
  This class holds a few data items and exposes them via QObject properties, so
  that they can be used in QML.  All data stored in this class is saved via
  QSettings on destruction.  Only one instance of this class should exist at any
  given time.

  The methods in this class are reentrant, but not thread safe.
*/

class GlobalSettings : public QObject
{
    Q_OBJECT

private:
    Q_DISABLE_COPY(GlobalSettings)

public:
    // Delete the move constructor
    GlobalSettings& operator=(GlobalSettings&&) = delete;

    // Delete the copy assignment constructor
    GlobalSettings(GlobalSettings&&) = delete;

    /*! \brief Standard constructor */
    explicit GlobalSettings(QObject *parent = nullptr);

    /*! \brief Standard deconstructor */
    ~GlobalSettings() override;

    /*! \brief Find out if Terms & Conditions have been accepted

    This property says which version of our "terms and conditions" have been
    accepted by the user; this is used to determine whether the first-use-dialog
    should be shown.  If nothing has been accepted yet, 0 is returned.
  */
    Q_PROPERTY(int acceptedTerms READ acceptedTerms WRITE setAcceptedTerms NOTIFY acceptedTermsChanged)

    /*! \brief Getter function for property of the same name */
    int acceptedTerms();

    /*! \brief Setter function for property of the same name */
    void setAcceptedTerms(int);

    /*! \brief Find out if "What's new" should be shown

    This property says if the dialog "What's new" should be shown on startup.
  */
    Q_PROPERTY(bool showWhatsNew READ showWhatsNew CONSTANT)

    /*! \brief Getter function for property of the same name */
    bool showWhatsNew();

    /*! \brief Hide airspaces with lower bound FL100 or above */
    Q_PROPERTY(bool hideUpperAirspaces READ hideUpperAirspaces WRITE setHideUpperAirspaces NOTIFY hideUpperAirspacesChanged)

    /*! \brief Getter function for property of the same name */
    bool hideUpperAirspaces();

    /*! \brief Setter function for property of the same name */
    void setHideUpperAirspaces(bool);

    /*! \brief Hide airspaces with lower bound FL100 or above */
    Q_PROPERTY(bool keepScreenOn READ keepScreenOn WRITE setKeepScreenOn NOTIFY keepScreenOnChanged)

    /*! \brief Getter function for property of the same name */
    bool keepScreenOn();

    /*! \brief Setter function for property of the same name */
    void setKeepScreenOn(bool);

signals:
    void acceptedTermsChanged();
    void hideUpperAirspacesChanged();
    void keepScreenOnChanged();

private:
    QSettings *settings;
};

#endif // satNav_H
