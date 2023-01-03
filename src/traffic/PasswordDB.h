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

#include <QHash>
#include <QQmlEngine>

#include "GlobalObject.h"

namespace Traffic {


/*! \brief Password database
 *
 *  This simple class provides access to a password database, which is in
 *  essence a glorified QHash<QString, QString>, where keys are network SSIDs
 *  and values are passwords.
 */
class PasswordDB : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    /*! \brief Default constructor
     *
     *  This default constructor will read the database from a file into memory.
     *
     *  @param parent The standard QObject parent pointer
     */
    PasswordDB(QObject* parent=nullptr);

    // No default constructor, important for QML singleton
    explicit PasswordDB() = delete;

    ~PasswordDB() override = default;

    // factory function for QML singleton
    static Traffic::PasswordDB* create(QQmlEngine*, QJSEngine*)
    {
        return GlobalObject::passwordDB();
    }


    //
    // Properties
    //

    /*! \brief Empty
     *
     *  This proerty contains true if the password database is empty.
     */
    Q_PROPERTY(bool empty READ empty NOTIFY emptyChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property empty
     */
    [[nodiscard]] auto empty() const -> bool
    {
        return m_empty;
    }


    //
    // Methods
    //

    /*! \brief Clear database
     *
     *  This method clears the database and saves it to the disk.
     */
    Q_INVOKABLE void clear();

    /*! \brief Check if database contains a given key
     *
     *  @param key Key to look up
     *
     *  @returns True if database contains the key
     */
    Q_INVOKABLE [[nodiscard]] bool contains(const QString& key) const
    {
        return m_passwordDB.contains(key);
    }

    /*! \brief Find password for a given key
     *
     *  @param key Key to look up
     *
     *  @returns Password, or an empty string if the database does not contain
     *  the key
     */
    Q_INVOKABLE [[nodiscard]] QString getPassword(const QString& key) const
    {
        return m_passwordDB.value(key);
    }

    /*! \brief Remove key/password
     *
     *  This method removes a key/password from the database and saves the
     *  database to the disk.
     *
     *  @param key Key for password to be removed.
     */
    Q_INVOKABLE void removePassword(const QString& key);

    /*! \brief Set key/password
     *
     *  This method adds a key/password to the database and saves the database
     *  to the disk.  If a password already exists for the given key, then it
     *  will be overwritten.
     *
     *  @param key Key to be added
     *
     *  @param password Password to be added
     */
    Q_INVOKABLE void setPassword(const QString& key, const QString& password);

signals:
    /*! \brief Notifier signal */
    void emptyChanged();

private:
    // Update the property 'empty' and emit the notifier signal, if appropriate
    void updateEmpty();

    // Save database to disk
    void save();

    // Property empty
    bool m_empty {true};

    // Password database
    QString passwordDBFileName {};

    QHash<QString, QString> m_passwordDB {};
};

} // namespace Traffic
