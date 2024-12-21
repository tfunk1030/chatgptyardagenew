#pragma once

#include <cmath>

namespace gptgolf::physics {

/**
 * @brief A 3D vector class for physics calculations
 * 
 * This class provides basic 3D vector operations needed for trajectory calculations,
 * including vector addition, subtraction, scaling, and magnitude calculations.
 */
class Vector3D {
public:
    double x, y, z;

    /**
     * @brief Construct a new Vector3D object
     * 
     * @param x X component
     * @param y Y component
     * @param z Z component (defaults to 0)
     */
    Vector3D(double x = 0.0, double y = 0.0, double z = 0.0)
        : x(x), y(y), z(z) {}

    /**
     * @brief Calculate the magnitude of the vector
     * 
     * @return double The magnitude (length) of the vector
     */
    double magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    /**
     * @brief Scale the vector by a scalar value
     * 
     * @param scalar The scaling factor
     * @return Vector3D The scaled vector
     */
    Vector3D scale(double scalar) const {
        return Vector3D(x * scalar, y * scalar, z * scalar);
    }

    /**
     * @brief Vector addition operator
     */
    Vector3D operator+(const Vector3D& other) const {
        return Vector3D(x + other.x, y + other.y, z + other.z);
    }

    /**
     * @brief Vector subtraction operator
     */
    Vector3D operator-(const Vector3D& other) const {
        return Vector3D(x - other.x, y - other.y, z - other.z);
    }
};

} // namespace gptgolf::physics
