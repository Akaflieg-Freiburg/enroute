#pragma once

#include <QObject>
#include <QHash>
#include <QString>

namespace Traffic {

/*! \brief Transponder database. 
 * This class implements an algorithm to decode the 24-bit transponder address to a callsign for some countries.
 * It is based on code from dump1090 at 
 * https://github.com/mutability/dump1090/blob/unmaintained/public_html/registrations.js
 * under GPLv2 license. 
 */
class TransponderDB : public QObject {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     *  This constructor initializes the database from a file or embedded data.
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit TransponderDB(QObject* parent = nullptr);

    /*! \brief Get registration for a given ICAO address
     *
     *  @param address The ICAO 24-bit transponder address (hexadecimal string).
     *
     *  @returns The aircraft registration, or an empty string if not found.
     */
    Q_INVOKABLE QString getRegistration(const QString& address) const;

private:
    QHash<QString, QString> m_database; // Mapping of ICAO addresses to registrations

    /*! \brief Load the database from an embedded resource or file */
    void loadDatabase();

    /*! \brief Decode fallback registration based on ICAO address ranges
     *
     *  @param address The ICAO 24-bit transponder address (hexadecimal string).
     *
     *  @returns The fallback registration, or an empty string if no match is found.
     */
    QString registration_from_hexid(const QString& address) const;
};

} // namespace Traffic
