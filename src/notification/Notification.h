/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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

#include <QQmlEngine>


namespace Notifications {


class Notification : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    //
    // Constructors and destructors
    //

    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Notification(QObject* parent = nullptr);

    // No default constructor, always want a parent
    explicit Notification() = delete;

    /*! \brief Standard destructor */
    ~Notification() = default;


    //
    // PROPERTIES
    //

    Q_PROPERTY(quint8 importance READ importance WRITE setImportance NOTIFY importanceChanged)
    quint8 importance() const { return m_importance; }
    void setImportance(quint8 newImportance);

    Q_PROPERTY(QString title READ title  WRITE setTitle NOTIFY titleChanged)
    QString title() const { return m_title; }
    void setTitle(const QString& newTitle);

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    QString text() const { return m_text; }
    void setText(const QString& newText);

    Q_PROPERTY(QString button1Text READ button1Text WRITE setButton1Text NOTIFY button1TextChanged)
    QString button1Text() const { return m_button1Text; }
    void setButton1Text(const QString& newButton1Text);

    Q_PROPERTY(QString button2Text READ button2Text WRITE setButton2Text NOTIFY button2TextChanged)
    QString button2Text() const { return m_button2Text; }
    void setButton2Text(const QString& newButton2Text);

public slots:
    virtual void button1Clicked();
    virtual void button2Clicked();

signals:
    void importanceChanged();
    void titleChanged();
    void textChanged();
    void button1TextChanged();
    void button2TextChanged();

private:
    QString m_title;
    QString m_text;
    QString m_button1Text;
    QString m_button2Text;
    quint8 m_importance {0};
};

} // namespace Notifications
