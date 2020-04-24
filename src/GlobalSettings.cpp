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

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>

#include "GlobalSettings.h"


GlobalSettings::GlobalSettings(QObject *parent)
    : QObject(parent)
{
    settings = new QSettings(this);

    _hasTranslation = QFile::exists(QString(":enroute_%1.qm").arg(QLocale::system().name().left(2)));
    qWarning() << "hasTranslation" << _hasTranslation;
#warning need to install translators
}


GlobalSettings::~GlobalSettings()
{
    // Remove translators
    setTranslate(false);
    delete enrouteTranslator;
    delete qtTranslator;

    // Save some values
    settings->setValue("lastVersion", PROJECT_VERSION);

    delete settings;
}


int GlobalSettings::acceptedTerms()
{
    return settings->value("acceptedTerms", 0).toInt();
}


void GlobalSettings::setAcceptedTerms(int terms)
{
    settings->setValue("acceptedTerms", terms);
    emit acceptedTermsChanged();
}


bool GlobalSettings::hideUpperAirspaces()
{
    return settings->value("Map/hideUpperAirspaces", false).toBool();
}


void GlobalSettings::setHideUpperAirspaces(bool hide)
{
    if (hide == hideUpperAirspaces())
        return;
    settings->setValue("Map/hideUpperAirspaces", hide);
    emit hideUpperAirspacesChanged();
}


bool GlobalSettings::keepScreenOn() const
{
    return settings->value("System/keepScreenOn", true).toBool();
}


void GlobalSettings::setKeepScreenOn(bool kso)
{
    if (kso == keepScreenOn())
        return;
    settings->setValue("System/keepScreenOn", kso);
    emit keepScreenOnChanged();
}


bool GlobalSettings::showWhatsNew()
{
    auto lastVersion = settings->value("lastVersion", "0.0.0").toString();
    if (lastVersion == "0.0.0")
        return false;
    return lastVersion != PROJECT_VERSION;
}

bool GlobalSettings::translate() const
{
    return settings->value("System/translate", true).toBool();
}

void GlobalSettings::setTranslate(bool trans)
{
    if (trans == translate())
        return;

    // Remove existing translators
    if (enrouteTranslator) {
        QCoreApplication::removeTranslator(enrouteTranslator);
        delete enrouteTranslator;
    }
    if (qtTranslator) {
        QCoreApplication::removeTranslator(qtTranslator);
        delete qtTranslator;
    }

    // If desired, install new translators
    if (trans) {
        enrouteTranslator = new QTranslator(this);
        auto a = enrouteTranslator->load(QString(":enroute_%1.qm").arg(QLocale::system().name().left(2)));
        QCoreApplication::installTranslator(enrouteTranslator);

        qtTranslator = new QTranslator(this);
        auto b = qtTranslator->load(QString("qt_%1.qm").arg(QLocale::system().name().left(2)), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
        QCoreApplication::installTranslator(qtTranslator);
        qWarning() << "install " << a << b;
    }

    settings->setValue("System/translate", trans);
    emit translateChanged();
}
