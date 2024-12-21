#include "weather/weather_data.h"
#include <cmath>

// Constants for air density calculations
const double DRY_AIR_GAS_CONSTANT = 287.05; // J/(kg·K)
const double WATER_VAPOR_GAS_CONSTANT = 461.495; // J/(kg·K)
const double REFERENCE_PRESSURE = 1013.25; // hPa (sea level)
const double REFERENCE_TEMPERATURE = 288.15; // K (15°C)

double calculateAirDensity(const WeatherData& data) {
    // Convert temperature to Kelvin
    double tempK = data.temperature + 273.15;
    
    // Calculate saturation vapor pressure using Magnus formula
    double satVaporPressure = 6.1121 * std::exp((18.678 - data.temperature/234.5) * 
                                               (data.temperature/(257.14 + data.temperature)));
    
    // Calculate actual vapor pressure using relative humidity
    double vaporPressure = (data.humidity/100.0) * satVaporPressure;
    
    // Calculate partial pressures
    double dryAirPressure = data.pressure - vaporPressure;
    
    // Calculate air density using the ideal gas law for mixture
    double dryAirDensity = dryAirPressure * 100 / (DRY_AIR_GAS_CONSTANT * tempK);
    double waterVaporDensity = vaporPressure * 100 / (WATER_VAPOR_GAS_CONSTANT * tempK);
    
    return dryAirDensity + waterVaporDensity;
}

double calculateWindEffect(const WeatherData& data) {
    // Convert wind speed to account for air density variation
    double densityRatio = calculateAirDensity(data) / 1.225; // Compare to standard air density
    
    // Adjust wind effect based on density ratio
    return data.windSpeed * std::sqrt(densityRatio);
}

double applyAltitudeAdjustment(double value, double altitude) {
    // Use international barometric formula
    double pressureRatio = std::pow(1 - 0.0065 * altitude / REFERENCE_TEMPERATURE, 5.2561);
    double temperatureRatio = 1 - 0.0065 * altitude / REFERENCE_TEMPERATURE;
    
    // Adjust value based on altitude effects
    return value * std::sqrt(pressureRatio / temperatureRatio);
}
