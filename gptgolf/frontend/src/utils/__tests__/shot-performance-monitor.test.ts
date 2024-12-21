import { performanceMonitor } from '../shot-performance-monitor';

describe('ShotPerformanceMonitor', () => {
  beforeEach(() => {
    performanceMonitor.clearMetrics();
  });

  it('records and retrieves metrics correctly', () => {
    const startTime = performanceMonitor.startMeasurement();
    const metrics = performanceMonitor.recordMetrics(startTime, 100);

    expect(metrics).toHaveProperty('calculationTime');
    expect(metrics).toHaveProperty('memoryUsage');
    expect(metrics).toHaveProperty('pointCount', 100);
    expect(metrics).toHaveProperty('timestamp');
  });

  it('calculates performance stats correctly', () => {
    // Record multiple metrics
    for (let i = 0; i < 5; i++) {
      const startTime = performanceMonitor.startMeasurement();
      // Simulate some work
      for (let j = 0; j < 1000000; j++) {
        Math.random();
      }
      performanceMonitor.recordMetrics(startTime, 100 + i);
    }

    const stats = performanceMonitor.getStats();
    expect(stats.totalCalculations).toBe(5);
    expect(stats.avgPointCount).toBeGreaterThan(100);
    expect(stats.avgCalculationTime).toBeGreaterThan(0);
  });

  it('respects time window in stats calculation', () => {
    // Record metrics with different timestamps
    const now = Date.now();
    
    // Old metrics (5 seconds ago)
    jest.spyOn(Date, 'now').mockImplementation(() => now - 5000);
    performanceMonitor.recordMetrics(performanceMonitor.startMeasurement(), 100);
    
    // Recent metrics
    jest.spyOn(Date, 'now').mockImplementation(() => now);
    performanceMonitor.recordMetrics(performanceMonitor.startMeasurement(), 200);

    // Get stats for last 2 seconds
    const recentStats = performanceMonitor.getStats(2000);
    expect(recentStats.totalCalculations).toBe(1);
    expect(recentStats.avgPointCount).toBe(200);

    // Restore Date.now
    jest.restoreAllMocks();
  });

  it('calculates performance trends correctly', () => {
    // Record 15 metrics with increasing point counts
    for (let i = 0; i < 15; i++) {
      const startTime = performanceMonitor.startMeasurement();
      performanceMonitor.recordMetrics(startTime, 100 + i * 10);
    }

    const trends = performanceMonitor.getPerformanceTrends();
    
    expect(trends.calculationTimesTrend.length).toBeGreaterThan(0);
    expect(trends.memoryUsageTrend.length).toBeGreaterThan(0);
    expect(trends.pointCountTrend.length).toBeGreaterThan(0);
    
    // Point count should show an increasing trend
    const firstPointCount = trends.pointCountTrend[0];
    const lastPointCount = trends.pointCountTrend[trends.pointCountTrend.length - 1];
    expect(lastPointCount).toBeGreaterThan(firstPointCount);
  });

  it('handles threshold warnings correctly', () => {
    const consoleSpy = jest.spyOn(console, 'warn').mockImplementation();
    
    // Set low thresholds to trigger warnings
    performanceMonitor.setPerformanceThresholds({
      calculationTime: 0.1,
      memoryUsage: 1,
      pointCount: 50,
    });

    // Record metrics that exceed thresholds
    const startTime = performanceMonitor.startMeasurement();
    // Simulate some work to ensure calculation time exceeds threshold
    for (let i = 0; i < 1000000; i++) {
      Math.random();
    }
    performanceMonitor.recordMetrics(startTime, 100);

    expect(consoleSpy).toHaveBeenCalled();
    expect(consoleSpy.mock.calls.some(call => 
      call[0].includes('exceeded threshold')
    )).toBe(true);

    consoleSpy.mockRestore();
  });

  it('maintains maximum stored metrics limit', () => {
    const maxMetrics = 1000;
    const extraMetrics = 100;

    // Record more than max allowed metrics
    for (let i = 0; i < maxMetrics + extraMetrics; i++) {
      const startTime = performanceMonitor.startMeasurement();
      performanceMonitor.recordMetrics(startTime, 100);
    }

    const stats = performanceMonitor.getStats();
    expect(stats.totalCalculations).toBe(maxMetrics);
  });

  it('estimates memory usage correctly', () => {
    const startTime = performanceMonitor.startMeasurement();
    const metrics = performanceMonitor.recordMetrics(startTime, 100, {
      additionalData: 'test data'
    });

    // Memory usage should account for points and additional data
    expect(metrics.memoryUsage).toBeGreaterThan(0);
    
    // Record metrics with more points should use more memory
    const metricsWithMorePoints = performanceMonitor.recordMetrics(
      performanceMonitor.startMeasurement(),
      200,
      { additionalData: 'test data' }
    );

    expect(metricsWithMorePoints.memoryUsage).toBeGreaterThan(metrics.memoryUsage);
  });
});
