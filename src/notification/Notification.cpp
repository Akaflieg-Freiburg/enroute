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

#include "notification/Notification.h"


//
// Constructors and destructors
//

Notifications::Notification::Notification(QObject* parent) : QObject(parent)
{
}


//
// Setter Methods
//

void Notifications::Notification::setImportance(quint8 newImportance)
{
    if (newImportance == m_importance)
    {
        return;
    }
    m_importance = newImportance;
    emit importanceChanged();

}

void Notifications::Notification::setTitle(const QString& newTitle)
{
    if (newTitle == m_title)
    {
        return;
    }
    m_title = newTitle;
    emit titleChanged();
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

void Notifications::Notification::button1Clicked()
{
    qWarning() << "Button 1 clicked";
    deleteLater();
}

void Notifications::Notification::button2Clicked()
{
    qWarning() << "Button 2 clicked";
    deleteLater();
}
