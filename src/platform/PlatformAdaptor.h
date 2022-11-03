/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

#include <QtGlobal>

#if defined(Q_OS_ANDROID)
#include "PlatformAdaptor_Android.h"
#endif

#if defined(Q_OS_IOS)
#include "PlatformAdaptor_iOS.h"
#endif

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
#include "PlatformAdaptor_Linux.h"
#endif

#if defined(Q_OS_MACOS)
#include "PlatformAdaptor_MacOS.h"
#endif

#if defined(Q_OS_WIN)
#include "PlatformAdaptor_Windows.h"
#endif
