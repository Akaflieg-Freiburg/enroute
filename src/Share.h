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

#ifndef SHARE_H
#define SHARE_H

#include <QObject>

#include "FlightRoute.h"
#include "shareutils/shareutils.hpp"

class Share : public QObject
{
    Q_OBJECT
public:
    explicit Share(FlightRoute* flightRoute, QObject *parent = nullptr);
    ~Share();

public slots:
    void share(QString gpx_content);
    void openWith(QString gpx_content);

    /// Used by shareUtils, called when a URL to a file is receieved
    void receivedURL(QString url);

// protected:
    /// Used by shareUtils in android builds. android requires explicit checking
    /// for incoming intents, so when the app becomes active we check for them.
    void checkPendingIntents();

private:
    ShareUtils* mShareUtils;
    FlightRoute* flightRoute;
    void share__(QString gpx_content, bool do_share);
};

#endif // SHARE_H

