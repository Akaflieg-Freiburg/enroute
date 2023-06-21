
#include <QJsonDocument>

namespace GeoMaps
{

class openAir
{
public:
    /*! \brief Check if file contains valid OpenAIR data
         *
         *  @param fileName Name of a file
         *
         *  @returns True if the file is likely to contain valid OpenAIR data.
         */
    static bool isValid(const QString &fileName);

    /*! \brief Reads a file in openAIR format and returns a GeoJSON document
     *
     *  @param fileName Name of the openAIR file
     *
     *  @param error Reference to a QStringList where error messages will be appended.
     *
     *  @return If no error messages were appended, returns a QJsonDocument with GeoJSON as specified
     *  in https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation.
     *  If error messages were appended, returns an empty QJsonDocument
     */
    static QJsonDocument parse(const QString& fileName, QStringList& errorList, QStringList& warningList);
};


} // namespace GeoMaps
