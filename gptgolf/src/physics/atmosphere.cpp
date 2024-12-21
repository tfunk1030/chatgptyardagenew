#include "physics/atmosphere.h"
#include <cmath>
#include <stdexcept>

namespace gptgolf {
namespace physics {

// Global instance
AtmosphericModel standardAtmosphere;

AtmosphericModel::AtmosphericModel() {
    // Initialize standard atmospheric layers (ISA model)
    // Format: altitude(m), temperature(K), pressure(Pa), lapse rate(K/m)
    layers = {
        AtmosphericLayer(0,     288.15,  101325.0,   -0.0065),  // Troposphere
        AtmosphericLayer(11000, 216.65,  22632.1,     0.0),     // Tropopause
        AtmosphericLayer(20000, 216.65,  5474.89,     0.001),   // Stratosphere 1
        AtmosphericLayer(32000, 228.65,  868.019,     0.0028),  // Stratosphere 2
        AtmosphericLayer(47000, 270.65,  110.906,     0.0),     // Stratopause
    };
}

int AtmosphericModel::findLayerIndex(double altitude) const {
    for (size_t i = 1; i < layers.size(); ++i) {
        if (altitude < layers[i].baseAltitude) {
            return i - 1;
        }
    }
    return layers.size() - 1;
}

const AtmosphericLayer& AtmosphericModel::getLayer(double altitude) const {
    return layers[findLayerIndex(altitude)];
}

double AtmosphericModel::getTemperature(double altitude) const {
    const AtmosphericLayer& layer = getLayer(altitude);
    double deltaH = altitude - layer.baseAltitude;
    return layer.temperature + layer.lapseRate * deltaH;
}

double AtmosphericModel::getPressure(double altitude) const {
    const AtmosphericLayer& layer = getLayer(altitude);
    double deltaH = altitude - layer.baseAltitude;
    double T = getTemperature(altitude);
    
    if (std::abs(layer.lapseRate) < 1e-10) {
        // Isothermal layer
        return layer.pressure * exp(-g * deltaH / (R * T));
    } else {
        // Layer with temperature gradient
        double exponent = -g / (R * layer.lapseRate);
        return layer.pressure * pow(T / layer.temperature, exponent);
    }
}

double AtmosphericModel::calculateVaporPressure(double temperature, double humidity) const {
    // Magnus formula for saturation vapor pressure
    double tempC = temperature - 273.15;  // Convert to Celsius
    double saturationPressure = 610.78 * exp((17.27 * tempC) / (tempC + 237.3));
    return (humidity / 100.0) * saturationPressure;
}

double AtmosphericModel::getDensity(double altitude, const weather::WeatherData* weatherData) const {
    double temperature = getTemperature(altitude);
    double pressure = getPressure(altitude);
    
    if (weatherData) {
        // Adjust for local conditions if weather data is available
        double localTemp = weatherData->temperature + 273.15;  // Convert to K
        double localPress = weatherData->pressure * 100.0;     // Convert hPa to Pa
        
        // Calculate correction factors
        double tempRatio = temperature / localTemp;
        double pressRatio = pressure / localPress;
        
        // Apply corrections
        temperature *= tempRatio;
        pressure *= pressRatio;
        
        // Account for humidity
        double vaporPressure = calculateVaporPressure(temperature, weatherData->humidity);
        double virtualTemp = temperature / (1.0 - 0.378 * vaporPressure / pressure);
        
        return pressure / (R * virtualTemp);
    }
    
    // Standard atmosphere calculation
    return pressure / (R * temperature);
}

} // namespace physics
} // namespace gptgolf
