/*!
 * MIT License
 *
 * Copyright (c) 2019 Tim Seemann
 * modified for enroute 2020 by Johannes Zellner
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Share.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>

#ifndef ANDROID
// on the desktop we share to a file dialog
// which provides saving the gpx route to disk.
#   include <QFileDialog>
#endif

Share::Share(FlightRoute* flightRoute_in, QObject *parent) : QObject(parent), flightRoute(flightRoute_in)
{
    mShareUtils = new ShareUtils(this);
    connect(mShareUtils, SIGNAL(fileUrlReceived(QString)), this, SLOT(receivedURL(QString)));

    // android requires you to use a subdirectory within the AppDataLocation for
    // sending and receiving files. Because of this, when necessary we do a deep
    // copy of the gpx content.
    //
    // We clear this directory on creation of the Share object -- even if the
    // app didn't exit gracefully, the directory is still cleared when starting
    // the app next time.
    //
#if defined(Q_OS_ANDROID)
    mShareUtils->clearTempDir();
#endif
}

Share::~Share()
{
    // NOTE: temporary files are removed when creating Share...
}

void Share::share__(QString gpx_content, bool do_share)
{

    QDateTime now = QDateTime::currentDateTimeUtc();
    QString fname = now.toString("yyyy-MM-dd_hh.mm.ss") + ".gpx";

#ifdef ANDROID
    // in Qt, resources are not stored absolute file paths, so in order to
    // share the content we save it to disk. We save these temporary files
    // when creating new Share objects.
    //
    auto filePath = mShareUtils->tempDir() + fname;
#else
    auto filePath = QFileDialog::getSaveFileName(nullptr, tr("Save"), fname, tr(""));
    if (filePath.isEmpty()) {
        qDebug() << "WARNING: save file name empty";
        return;
    }
#endif

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).debug() << "Share::share " << filePath;
        return;
    }

    QTextStream out(&file);
    out << gpx_content;
    out.flush();
    file.close();

#ifdef ANDROID
    QString mimeType = "application/gpx+xml";

    // a requestID tracks an intent with an identifier, useful if you have
    // multiple intents being sent at once. We use requestID = 0 here to have
    // a non-blocking call in QShareUtils.createCustomChooserAndStartActivity()
    //
    int requestID = 0;
    if (do_share == false) {
        mShareUtils->viewFile(filePath, tr("Open with"), mimeType, requestID);
    } else {
        mShareUtils->sendFile(filePath, tr("Send") /* title not used for send */, mimeType, requestID);
    }
#endif
}

void Share::share(QString gpx_content)
{
    share__(gpx_content, true);
}

void Share::openWith(QString gpx_content)
{
    share__(gpx_content, false);
}

void Share::receivedURL(QString url)
{
#if defined(Q_OS_ANDROID)
    flightRoute->fromGpx(url);
#endif // Q_OS_ANDROID
}

void Share::checkPendingIntents()
{
#if defined(Q_OS_ANDROID)
    mShareUtils->checkPendingIntents();
#endif // Q_OS_ANDROID
}
