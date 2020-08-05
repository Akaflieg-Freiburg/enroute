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
#include <QNetworkReply>
#include <QPointer>
#include <QXmlStreamReader>

#include "WeatherReport.h"


#warning Docu

class Meteorologist : public QObject {
    Q_OBJECT

public:
#warning Docu
    explicit Meteorologist(QNetworkAccessManager *networkAccessManager,
                           const QGeoCoordinate& position,
                           const QVariantList& steerpts,
                           QObject *parent = nullptr);

    // Standard destructor
    ~Meteorologist() override;

    Q_PROPERTY(QList<QObject*> reports READ reports NOTIFY reportsChanged)
    QList<QObject*> reports() const;

    Q_PROPERTY(bool processing READ processing NOTIFY processingChanged)
    bool processing() const { return _processing; }

    Q_INVOKABLE void update(const QGeoCoordinate& position, const QVariantList& steerpts);

signals:
    void reportsChanged();
    void processingChanged();
    void error(QString message);

private:
    Q_DISABLE_COPY_MOVE(Meteorologist)

    QPointer<QNetworkAccessManager> _networkAccessManager;
    QList<QPointer<QNetworkReply>> _replies;
    size_t _replyCount;
    size_t _replyTotal;
    bool _processing;

    QList<QPointer<WeatherReport>> _reports;

    QMultiMap<QString, QVariant> readReport(QXmlStreamReader &xml, const QString &type);
    void decode();

private slots:
    void downloadFinished();
};
