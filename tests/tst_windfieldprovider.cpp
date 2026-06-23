/***************************************************************************
 *   Copyright (C) 2026 by Quentin Bossard                                 *
 *                                                                         *
 *   GNU General Public License, version 3 or later.                       *
 ***************************************************************************/

#include <QTest>
#include <QtMath>

#include "weather/WindFieldProvider.h"

using namespace Qt::Literals::StringLiterals;

namespace {

constexpr double MPS_TO_KN = 1.94384;
constexpr double EPS = 0.05;

// A small, fully-populated field. Times are far in the future so the field is
// never "stale" during the test. u is in m/s; v is zero throughout.
//
// Per node, u[time][level]:
//   level 0 = 0 ft, level 1 = 10000 ft; time 0 = 12:00Z, time 1 = 13:00Z.
//   (47,7) and (48,7): u = [[-10,-30],[-30,-30]]   (varies in alt and time)
//   (47,8) and (48,8): u = [[-30,-30],[-30,-30]]   (for the lon-bilinear test)
const QByteArray fixture = R"({
  "reference_time":"2099-01-01T00:00:00Z",
  "times":["2099-01-01T12:00:00Z","2099-01-01T13:00:00Z"],
  "levels_ft":[0,10000],
  "grid":[
    {"lat":47.0,"lon":7.0,"u":[[-10,-30],[-30,-30]],"v":[[0,0],[0,0]]},
    {"lat":47.0,"lon":8.0,"u":[[-30,-30],[-30,-30]],"v":[[0,0],[0,0]]},
    {"lat":48.0,"lon":7.0,"u":[[-10,-30],[-30,-30]],"v":[[0,0],[0,0]]},
    {"lat":48.0,"lon":8.0,"u":[[-30,-30],[-30,-30]],"v":[[0,0],[0,0]]}
  ]
})";

QDateTime iso(const char* s) { return QDateTime::fromString(QString::fromLatin1(s), Qt::ISODate).toUTC(); }

// Compare two compass angles (degrees), treating 0 == 360.
bool angleClose(double a, double b)
{
    double d = std::fmod(std::fabs(a - b), 360.0);
    d = std::min(d, 360.0 - d);
    return d < EPS;
}

} // namespace


class TestWindFieldProvider : public QObject
{
    Q_OBJECT

private slots:
    void loads();
    void windFromUV_convention();
    void nodeLookup_exact();
    void bilinear_spatial();
    void altitude_interpolation();
    void time_interpolation();
    void windAtStep_snapsToNearest();
    void staleField_isUnusable();
};


void TestWindFieldProvider::loads()
{
    Weather::WindFieldProvider wfp;
    QVERIFY(wfp.loadFromJson(fixture));
    QVERIFY(wfp.hasData());
    QVERIFY(wfp.isUsable());          // times in the future ⇒ not stale
    QCOMPARE(wfp.levelsFt(), (QList<int>{0, 10000}));
    QCOMPARE(wfp.gridPoints().size(), 4);
}

void TestWindFieldProvider::windFromUV_convention()
{
    // Meteorological: u eastward, v northward; "direction from" = atan2(-u,-v).
    auto w1 = Weather::WindFieldProvider::windFromUV(-10, 0);   // from the east
    QVERIFY(angleClose(w1.directionFrom().toDEG(), 90));
    QVERIFY(qAbs(w1.speed().toKN() - 10) < EPS);

    auto w2 = Weather::WindFieldProvider::windFromUV(10, 0);    // from the west
    QVERIFY(angleClose(w2.directionFrom().toDEG(), 270));

    auto w3 = Weather::WindFieldProvider::windFromUV(0, 10);    // from the south
    QVERIFY(angleClose(w3.directionFrom().toDEG(), 180));

    auto w4 = Weather::WindFieldProvider::windFromUV(0, -10);   // from the north
    QVERIFY(angleClose(w4.directionFrom().toDEG(), 0));
}

void TestWindFieldProvider::nodeLookup_exact()
{
    // At an exact grid node / level / step, no interpolation: u = -10 m/s.
    Weather::WindFieldProvider wfp;
    QVERIFY(wfp.loadFromJson(fixture));
    auto w = wfp.windAt(47.0, 7.0, 0.0, iso("2099-01-01T12:00:00Z"));
    QVERIFY(qAbs(w.speed().toKN() - 10 * MPS_TO_KN) < EPS);
    QVERIFY(angleClose(w.directionFrom().toDEG(), 90)); // u<0 ⇒ from east
}

void TestWindFieldProvider::bilinear_spatial()
{
    // Midpoint in lon between u=-10 and u=-30 ⇒ -20 m/s.
    Weather::WindFieldProvider wfp;
    QVERIFY(wfp.loadFromJson(fixture));
    auto w = wfp.windAt(47.5, 7.5, 0.0, iso("2099-01-01T12:00:00Z"));
    QVERIFY(qAbs(w.speed().toKN() - 20 * MPS_TO_KN) < EPS);
    QVERIFY(angleClose(w.directionFrom().toDEG(), 90));
}

void TestWindFieldProvider::altitude_interpolation()
{
    // Node (47,7): level0 u=-10, level1 u=-30; at 5000 ft ⇒ -20 m/s.
    Weather::WindFieldProvider wfp;
    QVERIFY(wfp.loadFromJson(fixture));
    auto w = wfp.windAt(47.0, 7.0, 5000.0, iso("2099-01-01T12:00:00Z"));
    QVERIFY(qAbs(w.speed().toKN() - 20 * MPS_TO_KN) < EPS);
}

void TestWindFieldProvider::time_interpolation()
{
    // Node (47,7) level0: t0 u=-10, t1 u=-30; at 12:30 ⇒ -20 m/s.
    Weather::WindFieldProvider wfp;
    QVERIFY(wfp.loadFromJson(fixture));
    auto w = wfp.windAt(47.0, 7.0, 0.0, iso("2099-01-01T12:30:00Z"));
    QVERIFY(qAbs(w.speed().toKN() - 20 * MPS_TO_KN) < EPS);
}

void TestWindFieldProvider::windAtStep_snapsToNearest()
{
    // 12:30 is equidistant; nearestStep keeps the earlier step (12:00), so the
    // result is the t0 value (-10), NOT the time-interpolated -20.
    Weather::WindFieldProvider wfp;
    QVERIFY(wfp.loadFromJson(fixture));
    auto w = wfp.windAtStep(47.0, 7.0, 0.0, iso("2099-01-01T12:30:00Z"));
    QVERIFY(qAbs(w.speed().toKN() - 10 * MPS_TO_KN) < EPS);
}

void TestWindFieldProvider::staleField_isUnusable()
{
    // Same grid, but with times in the past ⇒ stale ⇒ unusable ⇒ invalid wind.
    const QByteArray pastFixture = QByteArray(fixture).replace("2099", "2000");
    Weather::WindFieldProvider wfp;
    QVERIFY(wfp.loadFromJson(pastFixture));
    QVERIFY(wfp.hasData());
    QVERIFY(wfp.isStale());
    QVERIFY(!wfp.isUsable());
    auto w = wfp.windAt(47.0, 7.0, 0.0, iso("2000-01-01T12:00:00Z"));
    QVERIFY(!w.speed().isFinite());
}


QTEST_MAIN(TestWindFieldProvider)
#include "tst_windfieldprovider.moc"
