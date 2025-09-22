/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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

#include <QTimer>
#include <utility>

#include "notification/Notification.h"

using namespace std::chrono_literals;


//
// Constructors and destructors
//

Notifications::Notification::Notification(QObject* parent)
    : QObject(parent)
{

}

Notifications::Notification::Notification(QString _title,
                                          Notifications::Notification::Importance _importance,
                                          QObject* parent)
    : QObject(parent),
    m_importance(_importance),
    m_title(std::move(_title))

{
    // Auto-delete this notification in five minutes.
    QTimer::singleShot(5min, this, &QObject::deleteLater);
}



//
// Getter Methods
//

QString Notifications::Notification::spokenText() const
{
    if (m_spokenText.isEmpty()) {
        return m_title;
    }
    return m_spokenText;
}



//
// Setter Methods
//

void Notifications::Notification::setButton1Text(const QString& newButton1Text)
{
    if (newButton1Text == m_button1Text)
    {
        return;
    }
    m_button1Text = newButton1Text;
    emit button1TextChanged();
}

void Notifications::Notification::setButton2Text(const QString& newButton2Text)
{
    if (newButton2Text == m_button2Text)
    {
        return;
    }
    m_button2Text = newButton2Text;
    emit button2TextChanged();
}

void Notifications::Notification::setReactionTime(Units::Timespan newReactionTime)
{
    if (newReactionTime == m_reactionTime)
    {
        return;
    }
    m_reactionTime = newReactionTime;
    emit reactionTimeChanged();
}

void Notifications::Notification::setSpokenText(const QString& newSpokenText)
{
    if (newSpokenText == m_spokenText)
    {
        return;
    }
    m_spokenText = newSpokenText;
    emit spokenTextChanged();
}

void Notifications::Notification::setText(const QString& newText)
{
    if (newText == m_text)
    {
        return;
    }
    m_text = newText;
    emit textChanged();
}

void Notifications::Notification::setTextBodyAction(Notifications::Notification::TextBodyAction newTextBodyAction)
{
    if (newTextBodyAction == m_textBodyAction)
    {
        return;
    }
    m_textBodyAction = newTextBodyAction;
    emit textBodyActionChanged();
}



//
// Methods
//

void Notifications::Notification::onButton1Clicked()
{
    deleteLater();
}

void Notifications::Notification::onButton2Clicked()
{
    deleteLater();
}
