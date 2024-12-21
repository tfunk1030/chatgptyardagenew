# Golf Physics API Documentation

## Core Physics Engine

### Trajectory Calculation
```cpp
TrajectoryResult calculateTrajectory(
    double initialSpeed,    // Initial ball speed in m/s
    double launchAngle,     // Launch angle in degrees
    double spinRate,        // Ball spin rate in rpm
    double windSpeed,       // Wind speed in m/s
    double windAngle       // Wind angle in degrees (0° = tailwind, 180° = headwind)
)
```

Returns a `TrajectoryResult` containing:
- Array of trajectory points (x, y coordinates)
- Total distance traveled
- Maximum height (apex)

Performance characteristics:
- Adaptive timestep: 0.0005s - 0.005s
- Memory usage: ~24 bytes per trajectory point
- Point decimation threshold: 0.1m

### Wind Effects
```cpp
void calculateRelativeVelocity(
    double velocityX,      // Ball velocity X component
    double velocityY,      // Ball velocity Y component
    double windSpeed,      // Wind speed in m/s
    double windAngle,      // Wind angle in degrees
    double& relativeVelX,  // Output: relative velocity X
    double& relativeVelY   // Output: relative velocity Y
)
```

### Environmental Adjustments
```cpp
double getAirDensity(const WeatherData* weatherData)
double getWindAdjustedSpeed(double speed, const WeatherData* weatherData)
```

## Performance Monitoring

### Performance Metrics
```typescript
interface PerformanceMetrics {
    calculationTime: number;   // Calculation time in ms
    memoryUsage: number;       // Memory usage in KB
    pointCount: number;        // Number of trajectory points
    timestamp: number;         // Unix timestamp
}
```

### Monitoring API
```typescript
class ShotPerformanceMonitor {
    startMeasurement(): number
    recordMetrics(startTime: number, pointCount: number, additionalData?: object): PerformanceMetrics
    getStats(timeWindow?: number): PerformanceStats
    getPerformanceTrends(): PerformanceTrends
    clearMetrics(): void
}
```

## Data Storage

### SQLite Schema
```sql
CREATE TABLE shots (
    id INTEGER PRIMARY KEY,
    timestamp INTEGER,
    initial_speed REAL,
    launch_angle REAL,
    spin_rate REAL,
    wind_speed REAL,
    wind_angle REAL,
    distance REAL,
    apex REAL
);

CREATE TABLE trajectory_points (
    shot_id INTEGER,
    x REAL,
    y REAL,
    sequence INTEGER,
    FOREIGN KEY(shot_id) REFERENCES shots(id)
);
```

## Integration Guidelines

### Physics Engine Integration
1. Include required headers:
```cpp
#include "physics/physics.h"
#include "physics/trajectory.h"
```

2. Initialize weather data:
```cpp
WeatherData weatherData{
    temperature: 20.0,  // °C
    pressure: 1013.25,  // hPa
    humidity: 50.0      // %
};
```

3. Calculate trajectory:
```cpp
TrajectoryResult result = calculateTrajectory(
    44.7,    // 100 mph initial speed
    12.0,    // Launch angle
    2500.0,  // Spin rate
    5.0,     // Wind speed
    45.0     // Wind angle
);
```

### Performance Monitoring Integration
1. Import the monitor:
```typescript
import { performanceMonitor } from '../utils/shot-performance-monitor';
```

2. Track performance:
```typescript
const startTime = performanceMonitor.startMeasurement();
// Perform calculations
const metrics = performanceMonitor.recordMetrics(startTime, pointCount);
```

## Error Handling

### Physics Engine Errors
- Division by zero protection in velocity calculations
- Infinite loop prevention with maximum iteration limit
- Numerical stability checks in force calculations

### Performance Monitor Errors
- Threshold warnings for excessive calculation time
- Memory usage alerts
- Point count warnings

## Testing Guidelines

### Unit Tests
```cpp
TEST_F(PhysicsTest, TrajectoryCalculation) {
    TrajectoryResult result = calculateTrajectory(44.7, 12.0, 2500.0, 0.0, 0.0);
    EXPECT_GT(result.distance, 0.0);
    EXPECT_GT(result.apex, 0.0);
}
```

### Performance Tests
```cpp
TEST_F(PerformanceTest, CalculationTime) {
    auto startTime = performanceMonitor.startMeasurement();
    // Test code
    auto metrics = performanceMonitor.recordMetrics(startTime, pointCount);
    EXPECT_LE(metrics.calculationTime, MAX_CALCULATION_TIME);
}
```

## Optimization Guidelines

### Physics Calculations
1. Use precomputed constants where possible
2. Implement adaptive timesteps based on velocity
3. Apply point decimation for memory optimization
4. Cache frequently used calculations

### Memory Management
1. Preallocate vectors with reasonable sizes
2. Use point decimation for trajectory storage
3. Clear unused metrics periodically
4. Monitor memory usage trends

## Maintenance Procedures

### Regular Maintenance
1. Monitor performance metrics
2. Clean up old trajectory data
3. Verify calculation accuracy
4. Update weather data

### Performance Tuning
1. Adjust timestep thresholds
2. Optimize point decimation
3. Update performance thresholds
4. Monitor memory usage

### Troubleshooting
1. Check performance monitor logs
2. Verify input parameters
3. Validate weather data
4. Review calculation results
