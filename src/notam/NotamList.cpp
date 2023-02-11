/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus  *
 *   stefan.kebekus@gmail.com  *
 * *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or *
 *   (at your option) any later version.   *
 * *
 *   This program is distributed in the hope that it will be useful,   *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of*
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the *
 *   GNU General Public License for more details.  *
 * *
 *   You should have received a copy of the GNU General Public License *
 *   along with this program; if not, write to the *
 *   Free Software Foundation, Inc.,   *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. *
 ***************************************************************************/

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>


#include "notam/NotamList.h"


NOTAM::NotamList::NotamList()
{

}


QString NOTAM::NotamList::summary() 
{
    QStringList results;

    if (m_notams.size() == 1)
    {
        results += ("One notam");
    }
    if (m_notams.size() > 1)
    {
        results += QString("%1 notams").arg(m_notams.size());
    }

    if (!m_retrieved.isValid() || (m_retrieved.addDays(1) <  QDateTime::currentDateTime()))
    {
        results += QString("Notam list possibly outdated");
    }

    return results.join(" • ");
}

QString NOTAM::NotamList::text() 
{
    QString result;
    foreach (auto notam, m_notams) {
        result += notam.m_text + "\n\n";
    }
    return "<pre>"+result+"</pre>";
}
