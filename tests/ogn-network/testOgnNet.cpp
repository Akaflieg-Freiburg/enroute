#include <QCoreApplication>
#include <QDebug>
#include "TrafficDataSource_Ogn.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    qDebug() << "Starting network test for TrafficDataSource_Ogn...";

    Traffic::TrafficDataSource_Ogn ognDataSource(false, "", 0);
    // QObject::connect(&ognDataSource, &Traffic::TrafficDataSource_Ogn::factorWithPosition, [](const Traffic::TrafficFactor_WithPosition &factor) {
    //     qDebug() << "Received traffic data with position:" << factor.positionInfo().coordinate();
    // });

    ognDataSource.connectToTrafficReceiver();

    return app.exec();
}
