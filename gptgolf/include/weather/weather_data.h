#ifndef WEATHER_DATA_H
#define WEATHER_DATA_H

#include <string>
#include <ctime>

/**
 * @file weather_data.h
 * @brief Weather data structures and calculations for golf shot analysis
 *
 * This module provides structures and functions for handling weather-related
 * data that affects golf ball flight. It includes functionality for storing
 * weather measurements, caching data, and calculating weather effects on
 * ball trajectory.
 */

namespace gptgolf {
namespace weather {

/**
 * @brief Comprehensive weather measurement data
 *
 * Contains all relevant weather measurements that can affect golf ball flight.
 * All measurements use SI units for consistency with physics calculations.
 */
struct WeatherData {
    double temperature;     //!< Ambient temperature (°C)
    double humidity;        //!< Relative humidity (0-100%)
    double pressure;        //!< Barometric pressure (hPa)
    double windSpeed;       //!< Wind speed (m/s)
    double windDirection;   //!< Wind direction (degrees, 0 = North, clockwise)
    double precipitation;   //!< Precipitation rate (mm/hr)
    double altitude;        //!< Altitude above sea level (m)
    std::time_t timestamp; //!< Time of measurement (Unix timestamp)

    /**
     * @brief Validates weather data values
     * @return true if all values are within reasonable ranges
     *
     * Checks:
     * - Temperature: -50°C to +50°C
     * - Humidity: 0% to 100%
     * - Pressure: 850hPa to 1100hPa
     * - Wind speed: 0 m/s to 40 m/s
     * - Wind direction: 0° to 360°
     * - Precipitation: >= 0 mm/hr
     * - Altitude: -500m to 5000m
     */
    bool isValid() const {
        return temperature >= -50.0 && temperature <= 50.0 &&
               humidity >= 0.0 && humidity <= 100.0 &&
               pressure >= 850.0 && pressure <= 1100.0 &&
               windSpeed >= 0.0 && windSpeed <= 40.0 &&
               windDirection >= 0.0 && windDirection < 360.0 &&
               precipitation >= 0.0 &&
               altitude >= -500.0 && altitude <= 5000.0;
    }
};

/**
 * @brief Cache for weather data to minimize API calls
 *
 * Stores weather data with timestamp for determining when refresh is needed.
 * Helps optimize performance by reducing unnecessary weather API calls while
 * ensuring data freshness.
 */
struct WeatherCache {
    WeatherData currentData;           //!< Currently cached weather data
    bool isValid;                      //!< Indicates if cached data is valid
    std::time_t lastUpdate;            //!< Time of last cache update
    static const int CACHE_DURATION = 900; //!< Cache duration (15 minutes in seconds)

    /**
     * @brief Check if cached data needs refreshing
     * @return true if cache is invalid or expired
     *
     * Cache is considered stale if:
     * - It's marked as invalid
     * - More than CACHE_DURATION seconds have passed since last update
     */
    bool needsRefresh() const {
        return !isValid || (std::time(nullptr) - lastUpdate) > CACHE_DURATION;
    }
};

/**
 * @brief Calculate air density based on weather conditions
 *
 * Uses the ideal gas law with humidity corrections to calculate air density.
 * This is crucial for accurate ball flight predictions.
 *
 * @param data Current weather conditions
 * @return Air density in kg/m³
 *
 * Formula used:
 * ρ = (P * M) / (R * T) * (1 - 0.378 * e/P)
 * where:
 * - P is pressure in Pa
 * - M is molar mass of air
 * - R is gas constant
 * - T is temperature in K
 * - e is water vapor pressure
 */
double calculateAirDensity(const WeatherData& data);

/**
 * @brief Calculate wind effect on ball flight
 *
 * Determines the effective force of wind on the ball, taking into account
 * air density variations due to temperature and pressure.
 *
 * @param data Current weather conditions
 * @return Wind effect coefficient (dimensionless)
 *
 * The coefficient is used to scale wind forces in trajectory calculations:
 * - 1.0 represents standard conditions
 * - >1.0 indicates increased wind effect
 * - <1.0 indicates decreased wind effect
 */
double calculateWindEffect(const WeatherData& data);

/**
 * @brief Apply altitude-based adjustments to calculations
 *
 * Adjusts various physical parameters based on altitude to account for
 * changes in air density and other atmospheric effects.
 *
 * @param value Base value to adjust
 * @param altitude Altitude in meters
 * @return Adjusted value
 *
 * Adjustments consider:
 * - Air density variation with altitude
 * - Temperature lapse rate
 * - Pressure changes
 */
double applyAltitudeAdjustment(double value, double altitude);

} // namespace weather
} // namespace gptgolf

#endif // WEATHER_DATA_H
