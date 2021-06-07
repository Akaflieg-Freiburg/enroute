/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

#include <QSettings>

#include "units/Speed.h"

class QQuickItem;

#warning doku!

class DemoRunner : public QObject {
    Q_OBJECT

public:
#warning doku!
    explicit DemoRunner(QObject *parent = nullptr);

    // Standard destructor
    ~DemoRunner() override = default;

signals:
    void resizeMainWindow(int w, int h);
    void saveImage(QString fileName);

private slots:
#warning doku!
    void run();

private:
    Q_DISABLE_COPY_MOVE(DemoRunner)
};
