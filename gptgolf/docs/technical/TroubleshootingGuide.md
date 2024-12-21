# Golf Physics Engine Troubleshooting Guide

## Quick Reference

### Common Issues and Solutions
| Issue | Likely Cause | Quick Solution |
|-------|-------------|----------------|
| Slow Calculations | High point count | Adjust timestep or enable decimation |
| Memory Warnings | Excessive trajectory points | Clear old metrics or adjust storage |
| Inaccurate Results | Weather data issues | Verify weather API connection |
| Display Glitches | React rendering | Clear cache and reload |
| Database Errors | Corrupted data | Run integrity check |

## Detailed Solutions

### Performance Issues

#### Slow Trajectory Calculations
```typescript
// Check current performance
const metrics = performanceMonitor.getStats();
if (metrics.avgCalculationTime > 2.0) {
    // Solutions:
    // 1. Increase minimum timestep
    // 2. Enable point decimation
    // 3. Reduce weather update frequency
}
```

**Steps to Resolve:**
1. Check performance monitor
2. Review recent changes
3. Verify input parameters
4. Adjust optimization settings

#### Memory Usage Alerts
```cpp
// Memory optimization settings
struct OptimizationSettings {
    bool enablePointDecimation = true;
    double decimationThreshold = 0.1;  // meters
    size_t maxPoints = 500;
};
```

**Resolution Steps:**
1. Clear old metrics
2. Enable point decimation
3. Adjust storage limits
4. Verify memory leaks

### Calculation Accuracy

#### Wind Effect Issues
```cpp
// Verify wind calculations
void validateWindEffects() {
    // Expected ranges
    const double MAX_WIND_EFFECT = 50.0;  // yards
    const double MIN_WIND_EFFECT = -50.0;  // yards
    
    // Check results within range
    if (windEffect > MAX_WIND_EFFECT || windEffect < MIN_WIND_EFFECT) {
        logError("Wind effect outside expected range");
    }
}
```

**Troubleshooting Steps:**
1. Verify wind data
2. Check angle calculations
3. Validate effect magnitude
4. Review historical data

#### Trajectory Anomalies
```cpp
// Trajectory validation
bool validateTrajectory(const TrajectoryResult& result) {
    // Check reasonable limits
    if (result.distance > 400.0 || result.apex > 200.0) {
        return false;
    }
    
    // Verify point sequence
    for (size_t i = 1; i < result.trajectory.size(); ++i) {
        if (result.trajectory[i].x < result.trajectory[i-1].x) {
            return false;
        }
    }
    
    return true;
}
```

**Resolution Process:**
1. Validate input parameters
2. Check physics constants
3. Review environmental data
4. Compare with known good results

### Frontend Issues

#### Display Problems
```typescript
// React component error boundary
class ShotCalculatorErrorBoundary extends React.Component {
    componentDidCatch(error: Error, info: React.ErrorInfo) {
        // Log error
        console.error('Shot calculator error:', error);
        
        // Reset state
        this.setState({ hasError: true });
    }
}
```

**Fixes:**
1. Clear browser cache
2. Update React components
3. Check browser console
4. Verify data flow

#### Visualization Glitches
```typescript
// SVG rendering check
function validateSVGRendering() {
    const svg = document.querySelector('.trajectory-view svg');
    if (!svg) {
        console.error('SVG container missing');
        return false;
    }
    
    const viewBox = svg.getAttribute('viewBox');
    if (!viewBox) {
        console.error('Invalid SVG viewBox');
        return false;
    }
    
    return true;
}
```

**Resolution Steps:**
1. Check SVG rendering
2. Verify data binding
3. Update view parameters
4. Clear component cache

### Database Issues

#### Data Corruption
```sql
-- Database integrity check
PRAGMA integrity_check;
PRAGMA foreign_key_check;

-- Fix common issues
BEGIN TRANSACTION;
    -- Remove orphaned points
    DELETE FROM trajectory_points 
    WHERE shot_id NOT IN (SELECT id FROM shots);
    
    -- Fix invalid timestamps
    UPDATE shots 
    SET timestamp = CURRENT_TIMESTAMP 
    WHERE timestamp IS NULL;
COMMIT;
```

**Recovery Process:**
1. Run integrity check
2. Backup current data
3. Fix corrupted entries
4. Verify relationships

#### Query Performance
```sql
-- Optimize common queries
CREATE INDEX IF NOT EXISTS idx_shots_timestamp ON shots(timestamp);
CREATE INDEX IF NOT EXISTS idx_trajectory_shot_id ON trajectory_points(shot_id);

-- Analyze query performance
EXPLAIN QUERY PLAN
SELECT s.*, COUNT(t.shot_id) as point_count
FROM shots s
LEFT JOIN trajectory_points t ON s.id = t.shot_id
WHERE s.timestamp > DATE('now', '-7 days')
GROUP BY s.id;
```

**Optimization Steps:**
1. Review indexes
2. Optimize queries
3. Update statistics
4. Clean old data

### API Integration

#### Weather Data Issues
```typescript
// Weather API validation
async function validateWeatherData(data: WeatherData): Promise<boolean> {
    // Check data ranges
    if (data.temperature < -50 || data.temperature > 50) {
        throw new Error('Temperature out of range');
    }
    
    if (data.windSpeed < 0 || data.windSpeed > 100) {
        throw new Error('Wind speed out of range');
    }
    
    return true;
}
```

**Troubleshooting Steps:**
1. Check API status
2. Verify credentials
3. Validate data ranges
4. Test connectivity

#### Data Synchronization
```typescript
// Sync status check
interface SyncStatus {
    lastSync: Date;
    pendingUpdates: number;
    errors: string[];
}

function checkSyncStatus(): SyncStatus {
    return {
        lastSync: new Date(localStorage.getItem('lastSync')),
        pendingUpdates: parseInt(localStorage.getItem('pendingUpdates') || '0'),
        errors: JSON.parse(localStorage.getItem('syncErrors') || '[]')
    };
}
```

**Resolution Process:**
1. Check sync status
2. Clear pending updates
3. Reset sync state
4. Verify data integrity

## System Health Checks

### Quick Diagnostic
```bash
#!/bin/bash
# quick_diagnostic.sh

# Check system resources
echo "Memory Usage:"
free -h

# Check disk space
echo "Disk Usage:"
df -h

# Check process status
echo "Process Status:"
ps aux | grep golfphysics

# Check log errors
echo "Recent Errors:"
tail -n 100 /var/log/golfphysics/error.log | grep ERROR
```

### Performance Baseline
```typescript
// Establish performance baseline
function checkPerformanceBaseline() {
    const baseline = {
        calculationTime: 2.0,  // ms
        memoryUsage: 4096,    // KB
        pointCount: 500
    };
    
    const current = performanceMonitor.getStats();
    
    return {
        calculationTimeRatio: current.avgCalculationTime / baseline.calculationTime,
        memoryUsageRatio: current.avgMemoryUsage / baseline.memoryUsage,
        pointCountRatio: current.avgPointCount / baseline.pointCount
    };
}
```

## Contact Support

### When to Escalate
- Calculation errors > 5%
- Performance degradation > 50%
- Data corruption
- System crashes

### Support Channels
- Email: support@golfphysics.com
- Emergency: +1-555-0123
- Issue Tracker: https://github.com/golfphysics/issues

### Required Information
1. Error messages
2. System logs
3. Performance metrics
4. Recent changes
5. Steps to reproduce
