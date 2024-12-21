#pragma once

/**
 * @file wind.h
 * @brief Advanced wind modeling for golf ball trajectory calculations
 *
 * This module implements various wind profile models to accurately simulate
 * wind effects on golf ball trajectories. It accounts for wind speed and
 * direction variations with height, terrain effects, and atmospheric boundary
 * layer characteristics.
 */

/**
 * @brief 3D point representation for position and velocity vectors
 */
struct Point3D {
    double x; //!< X coordinate (m)
    double y; //!< Y coordinate (m)
    double z; //!< Z coordinate (m)
    
    /**
     * @brief Construct a new Point3D object
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     */
    Point3D(double x = 0.0, double y = 0.0, double z = 0.0)
        : x(x), y(y), z(z) {}
};

/**
 * @brief Available wind profile models
 *
 * Different models for how wind speed and direction vary with height
 */
enum class WindProfile {
    CONSTANT,    //!< Constant wind speed with height (simplified model)
    LOGARITHMIC, //!< Logarithmic increase with height (standard boundary layer profile)
    POWER_LAW,   //!< Power law profile (commonly used in wind engineering)
    EKMAN_SPIRAL //!< Ekman spiral model (includes direction change with height)
};

/**
 * @brief Parameters describing terrain effects on wind profiles
 *
 * Different terrain types affect how wind speed varies with height
 * through surface roughness and other characteristics.
 */
struct TerrainParameters {
    double roughnessLength;   //!< Surface roughness length (z0) in meters
    double powerLawExponent;  //!< Alpha exponent for power law profile
    double referenceHeight;   //!< Reference height for measurements (typically 10m)
    
    /**
     * @name Preset Terrain Types
     * Factory methods for common terrain types with standard parameters
     * @{
     */
    
    /**
     * @brief Water surface parameters
     * @return TerrainParameters for water surfaces (very smooth)
     */
    static TerrainParameters Water() { return {0.0002, 0.10, 10.0}; }
    
    /**
     * @brief Open terrain parameters (grass, few obstacles)
     * @return TerrainParameters for open terrain
     */
    static TerrainParameters OpenTerrain() { return {0.03, 0.143, 10.0}; }
    
    /**
     * @brief Suburban area parameters
     * @return TerrainParameters for suburban areas
     */
    static TerrainParameters Suburban() { return {0.3, 0.22, 10.0}; }
    
    /**
     * @brief Urban area parameters
     * @return TerrainParameters for urban areas
     */
    static TerrainParameters Urban() { return {1.0, 0.33, 10.0}; }
    /** @} */
};

/**
 * @brief Advanced wind modeling class
 *
 * Models wind effects on golf ball trajectories using various wind profile
 * models and terrain characteristics. Accounts for height-dependent wind
 * speed and direction variations.
 */
class Wind {
public:
    /**
     * @brief Construct a new Wind object
     * @param speed Base wind speed at reference height (m/s)
     * @param direction Wind direction in degrees (0 = North, clockwise)
     * @param profile Wind profile model to use
     * @param terrain Terrain parameters affecting wind profile
     */
    Wind(double speed, double direction, 
         WindProfile profile = WindProfile::LOGARITHMIC,
         TerrainParameters terrain = TerrainParameters::OpenTerrain());
    
    /**
     * @brief Calculate wind effect on a ball at a specific position
     * @param position Current ball position
     * @param ballVelocity Current ball velocity magnitude (m/s)
     * @return Modified position accounting for wind effects
     *
     * Applies wind force to the ball based on:
     * - Local wind speed and direction at ball height
     * - Relative velocity between ball and wind
     * - Ball's aerodynamic characteristics
     */
    Point3D applyWindEffect(const Point3D& position, double ballVelocity) const;
    
    /**
     * @brief Get wind speed at a specific height
     * @param height Height above ground (m)
     * @return Wind speed at specified height (m/s)
     *
     * Uses the selected wind profile model to calculate local wind speed:
     * - Logarithmic: u(z) = u* / k * ln((z + z0) / z0)
     * - Power Law: u(z) = uref * (z/zref)^α
     * - Ekman: Includes both speed and direction changes
     */
    double getSpeedAtHeight(double height) const;
    
    /**
     * @brief Get wind direction at a specific height
     * @param height Height above ground (m)
     * @return Wind direction at specified height (degrees)
     *
     * Only the Ekman spiral model includes direction changes with height.
     * Other models return constant direction.
     */
    double getDirectionAtHeight(double height) const;
    
    /** @name Getters and Setters
     * Methods to access and modify wind parameters
     * @{
     */
    double getBaseSpeed() const;     //!< Get reference wind speed
    double getBaseDirection() const; //!< Get reference wind direction
    
    /**
     * @brief Set wind profile model
     * @param newProfile New wind profile to use
     */
    void setProfile(WindProfile newProfile);
    
    /**
     * @brief Get current wind profile model
     * @return Current wind profile
     */
    WindProfile getProfile() const;
    
    /**
     * @brief Set terrain parameters
     * @param newTerrain New terrain parameters
     */
    void setTerrain(const TerrainParameters& newTerrain);
    
    /**
     * @brief Get current terrain parameters
     * @return Current terrain parameters
     */
    const TerrainParameters& getTerrain() const;
    /** @} */

private:
    double speed;     //!< Reference wind speed in m/s
    double direction; //!< Reference wind direction in degrees
    WindProfile profile;
    TerrainParameters terrain;
    
    /**
     * @name Profile Calculations
     * Internal methods for different wind profile models
     * @{
     */
    double calculateLogProfile(double height) const;
    double calculatePowerLawProfile(double height) const;
    double calculateEkmanProfile(double height, double& outDirection) const;
    /** @} */
    
    /** @name Physical Constants
     * Constants used in wind profile calculations
     * @{
     */
    static constexpr double VON_KARMAN = 0.41;  //!< von Kármán constant
    static constexpr double CORIOLIS_PARAMETER = 1e-4;  //!< Typical mid-latitude value (s^-1)
    static constexpr double EKMAN_LAYER_HEIGHT = 1000.0;  //!< Typical height in meters
    /** @} */
};
