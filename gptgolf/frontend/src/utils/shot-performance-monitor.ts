interface PerformanceMetrics {
  calculationTime: number;
  memoryUsage: number;
  pointCount: number;
  timestamp: number;
}

interface PerformanceStats {
  avgCalculationTime: number;
  maxCalculationTime: number;
  avgMemoryUsage: number;
  maxMemoryUsage: number;
  avgPointCount: number;
  totalCalculations: number;
}

class ShotPerformanceMonitor {
  private static instance: ShotPerformanceMonitor;
  private metrics: PerformanceMetrics[] = [];
  private readonly maxStoredMetrics = 1000;
  private readonly performanceThresholds = {
    calculationTime: 5, // ms
    memoryUsage: 4096, // KB
    pointCount: 500,
  };

  private constructor() {}

  static getInstance(): ShotPerformanceMonitor {
    if (!ShotPerformanceMonitor.instance) {
      ShotPerformanceMonitor.instance = new ShotPerformanceMonitor();
    }
    return ShotPerformanceMonitor.instance;
  }

  startMeasurement(): number {
    return performance.now();
  }

  recordMetrics(
    startTime: number,
    pointCount: number,
    additionalData?: { [key: string]: any }
  ): PerformanceMetrics {
    const endTime = performance.now();
    const calculationTime = endTime - startTime;
    
    // Estimate memory usage based on point count and additional data
    const baseMemoryPerPoint = 24; // bytes (x, y coordinates + overhead)
    const estimatedMemory = (pointCount * baseMemoryPerPoint + 
      (additionalData ? JSON.stringify(additionalData).length : 0)) / 1024; // Convert to KB

    const metrics: PerformanceMetrics = {
      calculationTime,
      memoryUsage: estimatedMemory,
      pointCount,
      timestamp: Date.now(),
    };

    this.metrics.push(metrics);
    if (this.metrics.length > this.maxStoredMetrics) {
      this.metrics.shift();
    }

    this.checkPerformanceThresholds(metrics);
    return metrics;
  }

  private checkPerformanceThresholds(metrics: PerformanceMetrics): void {
    if (metrics.calculationTime > this.performanceThresholds.calculationTime) {
      console.warn(`Calculation time (${metrics.calculationTime.toFixed(2)}ms) exceeded threshold`);
    }
    if (metrics.memoryUsage > this.performanceThresholds.memoryUsage) {
      console.warn(`Memory usage (${metrics.memoryUsage.toFixed(2)}KB) exceeded threshold`);
    }
    if (metrics.pointCount > this.performanceThresholds.pointCount) {
      console.warn(`Point count (${metrics.pointCount}) exceeded threshold`);
    }
  }

  getStats(timeWindow?: number): PerformanceStats {
    let relevantMetrics = this.metrics;
    if (timeWindow) {
      const cutoffTime = Date.now() - timeWindow;
      relevantMetrics = this.metrics.filter(m => m.timestamp >= cutoffTime);
    }

    if (relevantMetrics.length === 0) {
      return {
        avgCalculationTime: 0,
        maxCalculationTime: 0,
        avgMemoryUsage: 0,
        maxMemoryUsage: 0,
        avgPointCount: 0,
        totalCalculations: 0,
      };
    }

    const stats = relevantMetrics.reduce((acc, metric) => {
      return {
        totalCalcTime: acc.totalCalcTime + metric.calculationTime,
        maxCalcTime: Math.max(acc.maxCalcTime, metric.calculationTime),
        totalMemory: acc.totalMemory + metric.memoryUsage,
        maxMemory: Math.max(acc.maxMemory, metric.memoryUsage),
        totalPoints: acc.totalPoints + metric.pointCount,
        count: acc.count + 1,
      };
    }, {
      totalCalcTime: 0,
      maxCalcTime: 0,
      totalMemory: 0,
      maxMemory: 0,
      totalPoints: 0,
      count: 0,
    });

    return {
      avgCalculationTime: stats.totalCalcTime / stats.count,
      maxCalculationTime: stats.maxCalcTime,
      avgMemoryUsage: stats.totalMemory / stats.count,
      maxMemoryUsage: stats.maxMemory,
      avgPointCount: stats.totalPoints / stats.count,
      totalCalculations: stats.count,
    };
  }

  getPerformanceTrends(): {
    calculationTimesTrend: number[];
    memoryUsageTrend: number[];
    pointCountTrend: number[];
  } {
    const windowSize = 10; // Calculate trends using moving averages
    const trends: {
      calculationTimesTrend: number[];
      memoryUsageTrend: number[];
      pointCountTrend: number[];
    } = {
      calculationTimesTrend: [],
      memoryUsageTrend: [],
      pointCountTrend: [],
    };

    for (let i = windowSize - 1; i < this.metrics.length; i++) {
      const metricsWindow: PerformanceMetrics[] = this.metrics.slice(i - windowSize + 1, i + 1);
      const calcTime = metricsWindow.reduce((sum, m) => sum + m.calculationTime, 0) / windowSize;
      const memUsage = metricsWindow.reduce((sum, m) => sum + m.memoryUsage, 0) / windowSize;
      const pointCount = metricsWindow.reduce((sum, m) => sum + m.pointCount, 0) / windowSize;
      
      trends.calculationTimesTrend.push(calcTime);
      trends.memoryUsageTrend.push(memUsage);
      trends.pointCountTrend.push(pointCount);
    }

    return trends;
  }

  clearMetrics(): void {
    this.metrics = [];
  }

  setPerformanceThresholds(thresholds: Partial<typeof this.performanceThresholds>): void {
    Object.assign(this.performanceThresholds, thresholds);
  }
}

export const performanceMonitor = ShotPerformanceMonitor.getInstance();
