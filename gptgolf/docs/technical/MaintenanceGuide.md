# Golf Physics Engine Maintenance Guide

## System Architecture Overview

### Core Components
1. Physics Engine (C++)
   - Trajectory calculation
   - Wind effects
   - Environmental adjustments

2. Frontend Application (React/TypeScript)
   - Shot calculator interface
   - Performance monitoring
   - Data visualization

3. Data Storage (SQLite)
   - Shot history
   - Performance metrics
   - User preferences

## Regular Maintenance Tasks

### Daily Checks
1. **Performance Monitoring**
   ```typescript
   // Check performance metrics
   const stats = performanceMonitor.getStats(24 * 60 * 60 * 1000); // Last 24 hours
   if (stats.avgCalculationTime > 2.0) {
       console.warn('Average calculation time exceeding threshold');
   }
   ```

2. **Error Logging**
   - Review error logs in `/var/log/golfphysics/`
   - Check for repeated errors or patterns
   - Verify error tracking system functionality

3. **Database Maintenance**
   ```sql
   -- Clean up old temporary data
   DELETE FROM trajectory_points WHERE timestamp < DATE('now', '-30 days');
   
   -- Optimize database
   VACUUM;
   ```

### Weekly Tasks
1. **Performance Analysis**
   - Review performance trends
   - Identify optimization opportunities
   - Update performance thresholds if needed

2. **Data Cleanup**
   ```sql
   -- Archive old shot data
   INSERT INTO shots_archive 
   SELECT * FROM shots 
   WHERE timestamp < DATE('now', '-90 days');
   
   -- Remove archived data
   DELETE FROM shots 
   WHERE timestamp < DATE('now', '-90 days');
   ```

3. **Cache Management**
   - Clear expired cache entries
   - Verify cache hit rates
   - Optimize cache settings

### Monthly Tasks
1. **System Updates**
   - Check for dependency updates
   - Review security patches
   - Update documentation

2. **Performance Optimization**
   - Analyze long-term performance trends
   - Adjust system parameters
   - Update optimization thresholds

3. **Database Optimization**
   ```sql
   -- Rebuild indexes
   REINDEX shots;
   REINDEX trajectory_points;
   
   -- Update statistics
   ANALYZE;
   ```

## Troubleshooting Guide

### Performance Issues

#### High Calculation Times
1. Check system load
   ```bash
   top -b -n 1
   ```

2. Verify memory usage
   ```bash
   free -m
   ```

3. Review recent changes
   ```bash
   git log --since="1 week ago" --pretty=format:"%h - %an, %ar : %s"
   ```

4. Profile calculations
   ```cpp
   // Enable detailed profiling
   #define ENABLE_PROFILING
   #include "profiler.h"
   
   void troubleshootCalculation() {
       PROFILE_FUNCTION();
       // Normal calculation code
   }
   ```

#### Memory Leaks
1. Use memory profiling tools
   ```bash
   valgrind --leak-check=full ./golf-physics
   ```

2. Check for resource cleanup
   ```cpp
   // Verify destructors
   ~TrajectoryCalculator() {
       clearCache();
       freeResources();
   }
   ```

3. Monitor memory trends
   ```typescript
   const memoryTrends = performanceMonitor.getPerformanceTrends().memoryUsageTrend;
   const increasing = memoryTrends.every((v, i, a) => 
       i === 0 || v >= a[i - 1]
   );
   ```

### Database Issues

#### Slow Queries
1. Analyze query performance
   ```sql
   EXPLAIN QUERY PLAN
   SELECT * FROM shots 
   WHERE timestamp > DATE('now', '-7 days');
   ```

2. Check indexes
   ```sql
   -- Add missing indexes
   CREATE INDEX IF NOT EXISTS idx_shots_timestamp 
   ON shots(timestamp);
   ```

3. Optimize queries
   ```sql
   -- Use specific columns instead of *
   SELECT id, timestamp, distance 
   FROM shots 
   WHERE timestamp > ?;
   ```

#### Data Corruption
1. Verify database integrity
   ```sql
   PRAGMA integrity_check;
   ```

2. Backup before repairs
   ```bash
   sqlite3 database.db ".backup 'backup.db'"
   ```

3. Repair if needed
   ```sql
   PRAGMA writable_schema = 1;
   DELETE FROM sqlite_master WHERE type = 'table' AND name = 'corrupted_table';
   PRAGMA writable_schema = 0;
   VACUUM;
   ```

## System Monitoring

### Performance Metrics
1. **Key Indicators**
   - Calculation time
   - Memory usage
   - Database response time
   - API latency

2. **Monitoring Setup**
   ```typescript
   // Set up monitoring thresholds
   performanceMonitor.setPerformanceThresholds({
       calculationTime: 2.0,    // ms
       memoryUsage: 4096,      // KB
       pointCount: 500
   });
   ```

3. **Alert Configuration**
   ```typescript
   // Configure alerts
   monitor.onThresholdExceeded((metric, value) => {
       notifyAdministrators(`${metric} exceeded: ${value}`);
   });
   ```

### Log Management
1. **Log Rotation**
   ```bash
   # /etc/logrotate.d/golfphysics
   /var/log/golfphysics/*.log {
       daily
       rotate 7
       compress
       delaycompress
       missingok
       notifempty
   }
   ```

2. **Log Analysis**
   ```bash
   # Search for errors
   grep -i error /var/log/golfphysics/app.log
   
   # Count occurrences
   grep -c "calculation failed" /var/log/golfphysics/app.log
   ```

## Backup Procedures

### Database Backups
1. **Daily Backups**
   ```bash
   #!/bin/bash
   # backup_db.sh
   DATE=$(date +%Y%m%d)
   sqlite3 database.db ".backup '/backups/db_${DATE}.db'"
   gzip "/backups/db_${DATE}.db"
   ```

2. **Verification**
   ```bash
   # Verify backup integrity
   sqlite3 backup.db "PRAGMA integrity_check;"
   ```

### Configuration Backups
1. **System Configuration**
   ```bash
   # Backup config files
   tar -czf /backups/config_$(date +%Y%m%d).tar.gz /etc/golfphysics/
   ```

2. **Application Settings**
   ```bash
   # Backup application settings
   cp -r /opt/golfphysics/config/* /backups/app_config/
   ```

## Recovery Procedures

### Database Recovery
1. **Restore from Backup**
   ```bash
   # Stop application
   systemctl stop golfphysics
   
   # Restore database
   sqlite3 database.db ".restore '/backups/db_backup.db'"
   
   # Start application
   systemctl start golfphysics
   ```

2. **Verify Recovery**
   ```sql
   -- Check data integrity
   SELECT COUNT(*) FROM shots;
   SELECT COUNT(*) FROM trajectory_points;
   ```

### System Recovery
1. **Configuration Restore**
   ```bash
   # Restore configs
   tar -xzf /backups/config_backup.tar.gz -C /
   
   # Reset permissions
   chown -R golfphysics:golfphysics /etc/golfphysics/
   ```

2. **Service Restart**
   ```bash
   systemctl restart golfphysics
   ```

## Contact Information

### Support Team
- Primary Contact: devops@golfphysics.com
- Emergency: oncall@golfphysics.com
- Hours: 24/7 for critical issues

### Escalation Path
1. Development Team Lead
2. System Administrator
3. CTO
4. CEO (Critical issues only)
