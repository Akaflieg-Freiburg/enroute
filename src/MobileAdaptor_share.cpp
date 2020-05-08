/***************************************************************************
 *   Copyright (C) 2020 by Johannes Zellner                                *
 *   johannes@zellner.org                                                  *
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


#include "MobileAdaptor.h"

#include <QDebug>

#if defined(Q_OS_ANDROID)
#include <QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>
#include <QAndroidJniEnvironment>
#endif


void MobileAdaptor::sendContent(const QString& content, const QString& mimeType, const QString& suffix)
{
  qWarning() << "MobileAdaptor::sendContent(" << content << ", " << mimeType << ", " << suffix << ")";
}
