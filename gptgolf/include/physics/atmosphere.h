#pragma once

#include <vector>
#include "weather/weather_data.h"

/**
 * @file atmosphere.h
 * @brief Atmospheric modeling for golf ball trajectory calculations
 *
 * This module implements the International Standard Atmosphere (ISA) model
 * for calculating atmospheric properties at different altitudes. These
 * calculations are crucial for accurate golf ball trajectory predictions
 * as they affect air density, drag, and lift forces.
 *
 * The model divides the atmosphere into layers with different temperature
 * lapse rates and base conditions, following the ISA standard.
 */

namespace gptgolf {
namespace physics {

/**
 * @brief Represents a layer in the International Standard Atmosphere
 *
 * The atmosphere is divided into layers with different properties.
 * Each layer has its own base conditions and temperature lapse rate,
 * used to calculate atmospheric properties within that layer.
 */
struct AtmosphericLayer {
    double baseAltitude;    //!< Base altitude of layer (m)
    double temperature;     //!< Base temperature at layer start (K)
    double pressure;        //!< Base pressure at layer start (Pa)
    double lapseRate;       //!< Temperature change with altitude (K/m)
    
    /**
     * @brief Construct a new Atmospheric Layer
     * @param alt Base altitude in meters
     * @param temp Base temperature in Kelvin
     * @param press Base pressure in Pascals
     * @param lapse Temperature lapse rate in K/m
     */
    AtmosphericLayer(double alt, double temp, double press, double lapse)
        : baseAltitude(alt), temperature(temp), pressure(press), lapseRate(lapse) {}
};

/**
 * @brief Models atmospheric conditions based on ISA standard
 *
 * Implements the International Standard Atmosphere model to calculate
 * atmospheric properties (temperature, pressure, density) at any altitude.
 * Can be adjusted for local weather conditions when available.
 */
class AtmosphericModel {
public:
    /**
     * @brief Initialize model with standard atmospheric layers
     *
     * Sets up the atmospheric layers according to the ISA standard:
     * - Troposphere: 0-11km, lapse rate -6.5 K/km
     * - Tropopause: 11-20km, isothermal
     * - Stratosphere: 20-32km, lapse rate +1.0 K/km
     */
    AtmosphericModel();
    
    /**
     * @brief Calculate temperature at given altitude
     * @param altitude Height above sea level (m)
     * @return Temperature in Kelvin
     *
     * Uses the layer's base temperature and lapse rate:
     * T = T_base + L * (h - h_base)
     * where L is the lapse rate
     */
    double getTemperature(double altitude) const;

    /**
     * @brief Calculate pressure at given altitude
     * @param altitude Height above sea level (m)
     * @return Pressure in Pascals
     *
     * For layers with non-zero lapse rate:
     * P = P_base * (T/T_base)^(-g/(R*L))
     *
     * For isothermal layers:
     * P = P_base * exp(-g*(h-h_base)/(R*T))
     */
    double getPressure(double altitude) const;

    /**
     * @brief Calculate air density at given altitude
     * @param altitude Height above sea level (m)
     * @param weatherData Optional local weather data to adjust calculations
     * @return Air density in kg/m³
     *
     * Uses the ideal gas law with humidity corrections:
     * ρ = P/(R*T) * (1 - 0.378*e/P)
     * where e is water vapor pressure
     */
    double getDensity(double altitude, const weather::WeatherData* weatherData = nullptr) const;
    
    /**
     * @brief Get atmospheric layer properties for given altitude
     * @param altitude Height above sea level (m)
     * @return Reference to the appropriate atmospheric layer
     * @throws std::out_of_range if altitude is beyond model limits
     */
    const AtmosphericLayer& getLayer(double altitude) const;
    
private:
    std::vector<AtmosphericLayer> layers; //!< Collection of atmospheric layers

    /** @name Physical Constants
     * Constants used in atmospheric calculations
     * @{
     */
    static constexpr double R = 287.058;   //!< Gas constant for dry air (J/(kg·K))
    static constexpr double g = 9.80665;   //!< Gravitational acceleration (m/s²)
    /** @} */
    
    /**
     * @brief Find the index of the layer containing given altitude
     * @param altitude Height above sea level (m)
     * @return Index of the appropriate layer
     * @throws std::out_of_range if altitude is beyond model limits
     */
    int findLayerIndex(double altitude) const;

    /**
     * @brief Calculate water vapor pressure from temperature and humidity
     * @param temperature Air temperature (K)
     * @param humidity Relative humidity (0-100%)
     * @return Water vapor pressure in Pascals
     *
     * Uses the Magnus formula:
     * e = RH * 611.2 * exp((17.62*T)/(243.12+T))
     * where T is temperature in Celsius
     */
    double calculateVaporPressure(double temperature, double humidity) const;
};

/**
 * @brief Global instance of the atmospheric model
 *
 * Provides convenient access to atmospheric calculations using
 * the International Standard Atmosphere model.
 */
extern AtmosphericModel standardAtmosphere;

} // namespace physics
} // namespace gptgolf
