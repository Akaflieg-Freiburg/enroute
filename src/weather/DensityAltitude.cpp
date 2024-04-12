#include "weather/DensityAltitude.h"
#include <cmath>

// calculate Density Altitude.
// Parameters:
//   elevation: The elevation above sea level in feet.
//   qnh: The atmospheric pressure at sea level in hPa.
//   T: The outside air temperature in degrees Celsius.
// For the formulas, see:
//   https://aerotoolbox.com/density-altitude/
double Weather::calculateDensityAltitude(double elevation, double qnh, double T) {
    double PA = (1013.25 - qnh) * 30.0 + elevation;
    double Tisa = 15.0 - (0.00198 * PA);
    double DA = PA + 120.0 * (T - Tisa);
    return DA;
}
