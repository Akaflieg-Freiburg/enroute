/* 
* This class implements an algorithm to decode the 24-bit transponder address to a callsign for some countries.
* It is based on code from dump1090 at 
* https://github.com/mutability/dump1090/blob/unmaintained/public_html/registrations.js
* under GPLv2 license. 
*/

#include "TransponderDB.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStringList>
#include <cmath>

namespace Traffic {

// Define alphabets
static const QString LIMITED_ALPHABET = "ABCDEFGHJKLMNPQRSTUVWXYZ"; // 24 chars; no I, O
static const QString FULL_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";  // 26 chars

// Define stride mappings
struct StrideMapping {
    uint32_t start;
    uint32_t s1;
    uint32_t s2;
    QString prefix;
    QString alphabet = FULL_ALPHABET;
    QString first;
    QString last;
    uint32_t offset = 0;
    uint32_t end = 0;
};

static QList<StrideMapping> strideMappings = {
    {0x008011, 26 * 26, 26, "ZS-"},
    {0x390000, 1024, 32, "F-G"},
    {0x398000, 1024, 32, "F-H"},
    {0x3C4421, 1024, 32, "D-A", FULL_ALPHABET, "AAA", "OZZ"},
    {0x3C0001, 26 * 26, 26, "D-A", FULL_ALPHABET, "PAA", "ZZZ"},
    {0x3C8421, 1024, 32, "D-B", FULL_ALPHABET, "AAA", "OZZ"},
    {0x3C2001, 26 * 26, 26, "D-B", FULL_ALPHABET, "PAA", "ZZZ"},
    {0x3CC000, 26 * 26, 26, "D-C"},
    {0x3D04A8, 26 * 26, 26, "D-E"},
    {0x3D4950, 26 * 26, 26, "D-F"},
    {0x3D8DF8, 26 * 26, 26, "D-G"},
    {0x3DD2A0, 26 * 26, 26, "D-H"},
    {0x3E1748, 26 * 26, 26, "D-I"},
    {0x448421, 1024, 32, "OO-"},
    {0x458421, 1024, 32, "OY-"},
    {0x460000, 26 * 26, 26, "OH-"},
    {0x468421, 1024, 32, "SX-"},
    {0x490421, 1024, 32, "CS-"},
    {0x4A0421, 1024, 32, "YR-"},
    {0x4B8421, 1024, 32, "TC-"},
    {0x740421, 1024, 32, "JY-"},
    {0x760421, 1024, 32, "AP-"},
    {0x768421, 1024, 32, "9V-"},
    {0x778421, 1024, 32, "YK-"},
    {0x7C0000, 1296, 36, "VH-"},
    {0xC00001, 26 * 26, 26, "C-F"},
    {0xC044A9, 26 * 26, 26, "C-G"},
    {0xE01041, 4096, 64, "LV-"}
};

// Initialize stride mappings
void initStrideMappings() {
    for (auto& mapping : strideMappings) {
        if (!mapping.first.isEmpty()) {
            int c1 = mapping.alphabet.indexOf(mapping.first[0]);
            int c2 = mapping.alphabet.indexOf(mapping.first[1]);
            int c3 = mapping.alphabet.indexOf(mapping.first[2]);
            mapping.offset = c1 * mapping.s1 + c2 * mapping.s2 + c3;
        } else {
            mapping.offset = 0;
        }

        if (!mapping.last.isEmpty()) {
            int c1 = mapping.alphabet.indexOf(mapping.last[0]);
            int c2 = mapping.alphabet.indexOf(mapping.last[1]);
            int c3 = mapping.alphabet.indexOf(mapping.last[2]);
            mapping.end = mapping.start - mapping.offset +
                          c1 * mapping.s1 +
                          c2 * mapping.s2 +
                          c3 - mapping.offset;
        } else {
            mapping.end = mapping.start - mapping.offset +
                          (mapping.alphabet.length() - 1) * mapping.s1 +
                          (mapping.alphabet.length() - 1) * mapping.s2 +
                          (mapping.alphabet.length() - 1);
        }
    }
}

// Lookup function for stride mappings
QString lookupStride(uint32_t hexid) {
    for (const auto& mapping : strideMappings) {
        if (hexid < mapping.start || hexid > mapping.end) {
            continue;
        }

        uint32_t offset = hexid - mapping.start + mapping.offset;

        int i1 = offset / mapping.s1;
        offset %= mapping.s1;
        int i2 = offset / mapping.s2;
        offset %= mapping.s2;
        int i3 = offset;

        if (i1 < 0 || i1 >= mapping.alphabet.length() ||
            i2 < 0 || i2 >= mapping.alphabet.length() ||
            i3 < 0 || i3 >= mapping.alphabet.length()) {
            continue;
        }

        return mapping.prefix + mapping.alphabet[i1] + mapping.alphabet[i2] + mapping.alphabet[i3];
    }

    return QString(); // No match found
}

// Define numeric mappings
struct NumericMapping {
    uint32_t start;
    uint32_t first;
    uint32_t count;
    QString templateString;
    uint32_t end = 0;
};

static QList<NumericMapping> numericMappings = {
    {0x140000, 0, 100000, "RA-00000"},
    {0x0B03E8, 1000, 1000, "CU-T0000"}
};

// Initialize numeric mappings
void initNumericMappings() {
    for (auto& mapping : numericMappings) {
        mapping.end = mapping.start + mapping.count - 1;
    }
}

// Lookup function for numeric mappings
QString lookupNumeric(uint32_t hexid) {
    for (const auto& mapping : numericMappings) {
        if (hexid < mapping.start || hexid > mapping.end) {
            continue;
        }

        uint32_t regNumber = hexid - mapping.start + mapping.first;
        QString reg = QString::number(regNumber);
        return mapping.templateString.left(mapping.templateString.length() - reg.length()) + reg;
    }

    return QString(); // No match found
}

// Constructor
TransponderDB::TransponderDB(QObject* parent) : QObject(parent) {
    loadDatabase();
    initStrideMappings();
    initNumericMappings();
}

// Get registration
QString TransponderDB::getRegistration(const QString& address) const {
    QString registration = m_database.value(address.toUpper(), QString());
    if (!registration.isEmpty()) {
        return registration;
    }

    bool ok;
    uint32_t hexid = address.toUInt(&ok, 16);
    if (!ok) {
        return QString();
    }

    // Try stride mappings
    registration = lookupStride(hexid);
    if (!registration.isEmpty()) {
        return registration;
    }

    // Try numeric mappings
    registration = lookupNumeric(hexid);
    if (!registration.isEmpty()) {
        return registration;
    }

    return QString(); // No match found
}

// Load database
void TransponderDB::loadDatabase() {
    QFile file(":/data/transponder_codes.txt"); // Replace with your data source
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to load transponder database.";
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.isEmpty() || line.startsWith('#')) {
            continue; // Skip empty lines and comments
        }

        QStringList parts = line.split(',');
        if (parts.size() == 2) {
            QString address = parts[0].trimmed().toUpper();
            QString registration = parts[1].trimmed();
            m_database.insert(address, registration);
        }
    }

    qDebug() << "Loaded" << m_database.size() << "transponder codes.";
}

} // namespace Traffic
