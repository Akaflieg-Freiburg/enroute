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

#include "WeatherReport_METAR.h"


WeatherReport::METAR::METAR(QXmlStreamReader &xml, QObject *parent) : QObject(parent)
{

    // Lambda to read sky condition
    auto readSky = [&] {
        QXmlStreamAttributes atrs = xml.attributes();
        QString sky;
        sky += atrs.value("sky_cover").toString();
        if (atrs.value("cloud_base_ft_agl").toString() != "")
            sky += "," + atrs.value("cloud_base_ft_agl").toString();
        if (atrs.value("cloud_type").toString() != "")
            sky += "," + atrs.value("cloud_type").toString();
        return sky;
    };

    // Read METAR/TAF and store in map
    QList<QString> accepted; // fields to decode without special treatment
    accepted = {"raw_text", "station_id", "observation_time", "temp_c",
                "dewpoint_c", "wind_dir_degrees", "wind_speed_kt",
                "wind_gust_kt", "visibility_statute_mi", "altim_in_hg",
                "flight_category", "wx_string"};

    while (true) {
        xml.readNextStartElement();
        QString name = xml.name().toString();

        if (xml.isStartElement() && accepted.contains(name) )
            data.insert(name, xml.readElementText());
        else if (xml.isStartElement() && name == "sky_condition") {
            data.insert("sky_condition", readSky());
            xml.skipCurrentElement();
        }
        else if (xml.isEndElement() && name == "METAR")
            break;
        else
            xml.skipCurrentElement();
    }

}

