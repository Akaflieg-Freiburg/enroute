/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

#include <QObject>
#include <QGeoCoordinate>
#include <QXmlStreamReader>

#include "WeatherReport.h"


#warning Docu

class Meteorologist : public QObject {
    Q_OBJECT

public:
#warning Docu
    explicit Meteorologist(const QGeoCoordinate& position, QObject *parent = nullptr);

    // Standard destructor
    ~Meteorologist() override = default;

    Q_PROPERTY(bool updated READ updated NOTIFY reportsChanged)
    bool updated() const { return _updated; }

    Q_PROPERTY(QList<QObject*> reports READ reports NOTIFY reportsChanged)
    QList<QObject*> reports() const;

    Q_INVOKABLE void update(const QGeoCoordinate& position);

signals:
    void reportsChanged();

private:
    Q_DISABLE_COPY_MOVE(Meteorologist)

    QList<QPointer<WeatherReport>> _reports;
    bool _updated;

    QMultiMap<QString, QVariant> _readReport(QXmlStreamReader &xml, const QString &type);

    QString dummyMetars();
    QString dummyTafs();
};
