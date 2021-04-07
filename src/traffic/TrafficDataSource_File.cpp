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

#include "traffic/TrafficDataSource_File.h"


// Member functions

Traffic::TrafficDataSource_File::TrafficDataSource_File(const QString& fileName, QObject *parent) :
    TrafficDataSource_Abstract(parent), simulatorFile(fileName) {

    connect(&simulatorTimer, &QTimer::timeout, this, &Traffic::TrafficDataSource_File::readFromSimulatorStream);

    // Initially, set properties
    updateProperties();
}


void Traffic::TrafficDataSource_File::connectToTrafficReceiver()
{
    // Do not do anything if the file is open and there are no errors
    if ( receivingHeartbeat() ) {
        return;
    }

    // Close the file if already open
    disconnectFromTrafficReceiver();

    // Open the file
    simulatorFile.unsetError();
    if (simulatorFile.open(QIODevice::ReadOnly)) {
        simulatorTextStream.setDevice(&simulatorFile);
        simulatorTextStream.setCodec("ISO 8859-1");
        lastPayload = QString();
        lastTime = 0;
        readFromSimulatorStream();
    }

    // Update properties
    updateProperties();
}


void Traffic::TrafficDataSource_File::disconnectFromTrafficReceiver()
{
    // Stop any simulation that might be running
    simulatorFile.close();
    simulatorTimer.stop();

    // Update properties
    setReceivingHeartbeat(false);
    updateProperties();
}


void Traffic::TrafficDataSource_File::readFromSimulatorStream()
{
    if ((simulatorFile.error() != QFileDevice::NoError) || simulatorTextStream.atEnd()) {
        disconnectFromTrafficReceiver();
        return;
    }

    if (!lastPayload.isEmpty()) {
        processFLARMSentence(lastPayload);
    }

    // Read line
    QString line;
    if (!simulatorTextStream.readLineInto(&line)) {
        disconnectFromTrafficReceiver();
        return;
    }

    // Set lastPayload, set timer
    auto tuple = line.split(QStringLiteral(" "));
    if (tuple.length() < 2) {
        return;
    }
    auto time = tuple[0].toInt();
    lastPayload = tuple[1];

    if (lastTime == 0) {
        simulatorTimer.setInterval(0);
    } else {
        simulatorTimer.setInterval(time-lastTime);
    }
    simulatorTimer.start();
    lastTime = time;
}


void Traffic::TrafficDataSource_File::updateProperties()
{
    // Set new value: connectivityStatus
    if ( simulatorFile.isOpen() && (simulatorFile.error() == QFileDevice::NoError)) {
        setConnectivityStatus( tr("Connected.") );
    } else {
        setConnectivityStatus( tr("Not connected.") );
    }

    // Set new value: errorString
    switch (simulatorFile.error()) {
    case QFileDevice::NoError:
        setErrorString( QString() );
        break;
    case QFileDevice::ReadError:
        setErrorString( tr("An error occurred when reading from the file.") );
        break;
    case QFileDevice::WriteError:
        setErrorString( tr("An error occurred when writing to the file.") );
        break;
    case QFileDevice::FatalError:
        setErrorString( tr("A fatal error occurred.") );
        break;
    case QFileDevice::ResourceError:
        setErrorString( tr("Out of resources (e.g., too many open files, out of memory, etc.)") );
        break;
    case QFileDevice::OpenError:
        setErrorString( tr("The file could not be opened.") );
        break;
    case QFileDevice::AbortError:
        setErrorString( tr("The operation was aborted.") );
        break;
    case QFileDevice::TimeOutError:
        setErrorString( tr("A timeout occurred.") );
        break;
    case QFileDevice::UnspecifiedError:
        setErrorString( tr("An unspecified error occurred.") );
        break;
    case QFileDevice::RemoveError:
        setErrorString( tr("The file could not be removed.") );
        break;
    case QFileDevice::RenameError:
        setErrorString( tr("The file could not be renamed.") );
        break;
    case QFileDevice::PositionError:
        setErrorString( tr("The position in the file could not be changed.") );
        break;
    case QFileDevice::ResizeError:
        setErrorString( tr("The file could not be resized.") );
        break;
    case QFileDevice::PermissionsError:
        setErrorString( tr("The file could not be accessed.") );
        break;
    case QFileDevice::CopyError:
        setErrorString( tr("The file could not be copied.") );
        break;
    }

}
