/***************************************************************************
 *   Copyright (C) 2023-2024 by Stefan Kebekus                             *
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
#include <QQmlEngine>

#include "units/Timespan.h"

using namespace Qt::Literals::StringLiterals;

namespace Notifications {

/*! \brief Base class for all notifications
 *
 *  This is the base class for all notifications. This notifications sets
 *  button1Text to "Dismiss", button2Text to "Empty" and reacts to clicks with a
 *  deleteLater(). More elaborate notifications can inherit from this class and
 *  re-implement some of the virtual methods.
 *
 *  @note Instance of this class may safely call deleteLater() at any time to
 *  indicate that the notification is no longer relevant.
 */

class Notification : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    /*! \brief Importance classification */
    enum Importance : quint8 {
        Info = 1<<1,               /*!< \brief Generic information, "Map updates available" */
        Info_Navigation = 1<<2,    /*!< \brief Information pertaining to navigation, "Top of descent reached" */
        Warning = 1<<3,            /*!< \brief Generic warning, "Traffic data receiver inop" */
        Warning_Navigation = 1<<4, /*!< \brief Serious warning, "Prohibited Airspace 1 minutes ahead" */
        Alert = 1<<5               /*!< \brief Alert. Immediate action is required to avoid accident or serious rule infringion */
    };
    Q_ENUM(Importance);

    /*! \brief Text body action
     *
     *  Describes the action that the GUI should take then the text body of
     *  this notification is clicked.
     */
    enum TextBodyAction : quint8 {
        None,                   /*!< \brief No action */
        OpenMapsAndDataPage,    /*!< \brief Open the page 'maps and data' */
        OpenTrafficReceiverPage /*!< \brief Open the page 'traffic receiver' */
    };
    Q_ENUM(TextBodyAction);


    //
    // Constructors and destructors
    //

public:
    /*! \brief Standard constructor
     *
     *  The constructor sets a time that calls deleteLater() after five minutes.
     *  This way, notifications are never shown permanently.
     *
     *  @param _title Notification title
     *
     *  @param _importance Notification importance
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit Notification(
        QString _title,
        Notifications::Notification::Importance _importance = Notifications::Notification::Info,
        QObject* parent = nullptr);

    // Standard constructor, required by QML
    explicit Notification(QObject* parent = nullptr);

    /*! \brief Standard destructor */
    ~Notification() override = default;

    //
    // PROPERTIES
    //

    /*! \brief Text for first button
     *
     *  This is the text string for the first button in the notification GUI. If
     *  empty, the button is invisible. The importance is initialized with an
     *  empty string.
     */
    Q_PROPERTY(QString button1Text READ button1Text WRITE setButton1Text NOTIFY button1TextChanged)

    /*! \brief Text for second button
     *
     *  This is the text string for the second button in the notification GUI.
     *  If empty, the button is invisible. The importance is initialized with an
     *  empty string.
     */
    Q_PROPERTY(QString button2Text READ button2Text WRITE setButton2Text NOTIFY button2TextChanged)

    /*! \brief Importance of the notification
     *
     *  The importance is used to choose the notification that is most relevant
     *  to the pilot. The importance is initialized with "Info".
     */
    Q_PROPERTY(Notifications::Notification::Importance importance READ importance CONSTANT)

    /*! \brief Time left to avert negative consequences
     *
     *  This property holds the time that the pilot has in order to avert
     *  potential negative consequences. This data is used to choose the
     *  notification that is most relevant to the pilot. The importance is
     *  initialized with qNaN.
     */
    Q_PROPERTY(Units::Timespan reactionTime READ reactionTime WRITE setReactionTime NOTIFY reactionTimeChanged)

    /*! \brief Spoken text message for the notification.
     *
     *  This property holds a text announcement which is spoken. When set to
     *  empty, a message is generated from the title and text properties. This
     *  property is initialized with an empty string
     */
    Q_PROPERTY(QString spokenText READ spokenText WRITE setSpokenText NOTIFY spokenTextChanged)

    /*! \brief Text body of the notification.
     *
     *  This property holds the text body, which can be empty.
     */
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

    /*! \brief Text body action
     *
     *  This property describes the action the that GUI should take when the
     *  text body is clicked. This property is initialized to 'None'.
     */
    Q_PROPERTY(Notifications::Notification::TextBodyAction textBodyAction READ textBodyAction WRITE setTextBodyAction NOTIFY textBodyActionChanged)

    /*! \brief Title of the notification.
     *
     *  This property holds the title, which should never be empty. This
     *  property is initialized with an enbarrassing placeholder string.
     */
    Q_PROPERTY(QString title READ title CONSTANT)


    //
    // Getter Methods
    //

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property button1Text
     */
    [[nodiscard]] QString button1Text() const { return m_button1Text; }

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property button1Text
     */
    [[nodiscard]] QString button2Text() const { return m_button2Text; }

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property importance
     */
    [[nodiscard]] Notifications::Notification::Importance importance() const { return m_importance; }

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property reactionTime
     */
    [[nodiscard]] Units::Timespan reactionTime() const { return m_reactionTime; }

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property spokenText
     */
    [[nodiscard]] QString spokenText() const;

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property text
     */
    [[nodiscard]] QString text() const { return m_text; }

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property textBodyAction
     */
    [[nodiscard]] Notifications::Notification::TextBodyAction textBodyAction() const { return m_textBodyAction; }

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property title
     */
    [[nodiscard]] QString title() const { return m_title; }



    //
    // Setter Methods
    //

    /*! \brief Setter function for property of the same name
     *
     *  @param newButton1Text Property button1Text
     */
    void setButton1Text(const QString& newButton1Text);

    /*! \brief Setter function for property of the same name
     *
     *  @param newButton2Text Property button1Text
     */
    void setButton2Text(const QString& newButton2Text);

    /*! \brief Setter function for property of the same name
     *
     *  @param newReactionTime Property reactionTime
     */
    void setReactionTime(Units::Timespan newReactionTime);

    /*! \brief Setter function for property of the same name
     *
     *  @param newSpokenText Property reactionTime
     */
    void setSpokenText(const QString& newSpokenText);

    /*! \brief Setter function for property of the same name
     *
     *  @param newText Property text
     */
    void setText(const QString& newText);

    /*! \brief Setter function for property of the same name
     *
     *  @param newTextBodyAction Property textBodyAction
     */
    void setTextBodyAction(Notifications::Notification::TextBodyAction newTextBodyAction);


    //
    // Methods
    //

    /*! \brief Mouse click handler
     *
     *  This method is called whenever the user clicks on the first button in
     *  the GUI representation of this notification. This implementation calls
     *  deleteLater(). Reimplement this method for proper reaction to GUI
     *  events.
     */
    Q_INVOKABLE virtual void onButton1Clicked();

    /*! \brief Mouse click handler
     *
     *  This method is called whenever the user clicks on the first button in
     *  the GUI representation of this notification. This implementation calls
     *  deleteLater(). Reimplement this method for proper reaction to GUI
     *  events.
     */
    Q_INVOKABLE virtual void onButton2Clicked();

signals:
    /*! \brief Notification signal */
    void button1TextChanged();

    /*! \brief Notification signal */
    void button2TextChanged();

    /*! \brief Notification signal */
    void reactionTimeChanged();

    /*! \brief Notification signal */
    void spokenTextChanged();

    /*! \brief Notification signal */
    void textChanged();

    /*! \brief Notification signal */
    void textBodyActionChanged();


private:
    Q_DISABLE_COPY_MOVE(Notification)

    // Property data
    QString m_button1Text {tr("Dismiss")};
    QString m_button2Text;
    Notifications::Notification::Importance m_importance {Info};
    Units::Timespan m_reactionTime;
    QString m_spokenText;
    QString m_title {u"Placeholder title"_s};
    QString m_text;
    Notifications::Notification::TextBodyAction m_textBodyAction {None};
};

} // namespace Notifications
