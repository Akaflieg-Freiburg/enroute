/***************************************************************************
 *   Copyright (C) 2024 by Stefan Kebekus                                  *
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

#include <QFile>
#include <QTextStream>

#include "fileFormats/CSV.h"

#include "fileFormats/DataFileAbstract.h"

//
// Private helper functions
//

QStringList FileFormats::CSV::parseCSV(const QString& string)
{
    // Thanks to https://stackoverflow.com/questions/27318631/parsing-through-a-csv-file-in-qt

    enum State
    {
        Normal,
        Quote
    } state = Normal;
    QStringList fields;
    fields.reserve(10);
    QString value;

    for (int i = 0; i < string.size(); i++)
    {
        const QChar current = string.at(i);

        // Normal state
        if (state == Normal)
        {
            // Comma
            if (current == ',')
            {
                // Save field
                fields.append(value.trimmed());
                value.clear();
            }

            // Double-quote
            else if (current == '"')
            {
                state = Quote;
                value += current;
            }

            // Other character
            else
            {
                value += current;
            }
        }

        // In-quote state
        else if (state == Quote)
        {
            // Another double-quote
            if (current == '"')
            {
                if (i < string.size())
                {
                    // A double double-quote?
                    if (i + 1 < string.size() && string.at(i + 1) == '"')
                    {
                        value += '"';

                        // Skip a second quote character in a row
                        i++;
                    }
                    else
                    {
                        state = Normal;
                        value += '"';
                    }
                }
            }

            // Other character
            else
            {
                value += current;
            }
        }
    }

    if (!value.isEmpty())
    {
        fields.append(value.trimmed());
    }

    // Quotes are left in until here; so when fields are trimmed, only whitespace outside of
    // quotes is removed.  The outermost quotes are removed here.
    for (auto &field : fields)
    {
        if (field.length() >= 1 && field.at(0) == '"')
        {
            field = field.mid(1);
            if (field.length() >= 1 && field.right(1) == '"')
            {
                field = field.left(field.length() - 1);
            }
        }
    }

    return fields;
}


FileFormats::CSV::CSV(const QString& fileName)
{
    auto file = FileFormats::DataFileAbstract::openFileURL(fileName);
    auto success = file->open(QIODevice::ReadOnly);
    if (!success)
    {
        setError(QObject::tr("Cannot open CSV file %1 for reading.", "FileFormats::CSV").arg(fileName));
        return;
    }

    QTextStream stream(file.data());
    QString line;
    stream.readLineInto(&line);
    while (stream.readLineInto(&line))
    {
        m_lines << parseCSV(line);
    }
}
