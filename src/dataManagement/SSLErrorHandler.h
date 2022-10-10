/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

#include <QNetworkReply>
#include <QSslError>

#include "GlobalObject.h"

namespace DataManagement
{

  /*! \brief Handles SSL error
   *
   *  This class watches for SSL errors that are reported by the global
   *  QNetworkAccessManager instance.  Depending on the settings, errors are
   *  either ignored or reported via the SSLError() signal.
   */

  class SSLErrorHandler : public GlobalObject
  {
    Q_OBJECT

  public:
    /*! \brief Standard constructor
     *
     *  @param parent The standard QObject parent pointer.
     */
    explicit SSLErrorHandler(QObject *parent = nullptr);

  signals:
    /*! \brief Notification signal for the property with the same name */
    void sslError(QString description);

  private slots:
    // This is the actual error handler.
    void onSSLError(QNetworkReply *reply, const QList<QSslError> &errors);

  private:
    Q_DISABLE_COPY_MOVE(SSLErrorHandler)

    // Re-implemented from base class. See documentation there.
    void deferredInitialization() override;
  };

};
