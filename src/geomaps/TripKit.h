
#pragma once

#include <QJsonArray>

#include "geomaps/Zip.h"

#warning implement
namespace GeoMaps
{
class TripKit {
public:
    TripKit(const QString& fileName);

    QString error() const { return m_error; }

    QString name() const { return m_name; }

    bool isValid() const { return m_error.isEmpty(); }

    void extract(const QString& directoryPath, int index);

    int numCharts() const { return m_charts.size(); }
private:
    GeoMaps::Zip m_zip;
    QString m_error;
    QString m_name;
    QJsonArray m_charts;
};

} // namespace GeoMaps
