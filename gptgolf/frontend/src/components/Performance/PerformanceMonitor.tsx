import React, { useEffect, useState } from 'react';
import { performanceMonitor } from '../../utils/shot-performance-monitor';
import styles from './PerformanceMonitor.module.css';

interface PerformanceStats {
  avgCalculationTime: number;
  maxCalculationTime: number;
  avgMemoryUsage: number;
  maxMemoryUsage: number;
  avgPointCount: number;
  totalCalculations: number;
}

interface PerformanceTrends {
  calculationTimesTrend: number[];
  memoryUsageTrend: number[];
  pointCountTrend: number[];
}

const PerformanceMonitor: React.FC = () => {
  const [stats, setStats] = useState<PerformanceStats | null>(null);
  const [trends, setTrends] = useState<PerformanceTrends | null>(null);
  const [updateInterval, setUpdateInterval] = useState<number>(1000);

  useEffect(() => {
    const intervalId = setInterval(() => {
      setStats(performanceMonitor.getStats());
      setTrends(performanceMonitor.getPerformanceTrends());
    }, updateInterval);

    return () => clearInterval(intervalId);
  }, [updateInterval]);

  const renderTrendGraph = (data: number[], label: string) => {
    if (!data.length) return null;

    const max = Math.max(...data);
    const min = Math.min(...data);
    const range = max - min;
    const height = 50; // Graph height in pixels

    const points = data.map((value, index) => {
      const x = (index / (data.length - 1)) * 100;
      const y = height - ((value - min) / range) * height;
      return `${x},${y}`;
    }).join(' ');

    return (
      <div className={styles.trendGraph}>
        <h4>{label}</h4>
        <svg width="100%" height={height} className={styles.graph}>
          <polyline
            points={points}
            className={styles.trendLine}
          />
        </svg>
        <div className={styles.trendValues}>
          <span>Min: {min.toFixed(2)}</span>
          <span>Max: {max.toFixed(2)}</span>
        </div>
      </div>
    );
  };

  if (!stats) {
    return <div>Loading performance data...</div>;
  }

  return (
    <div className={styles.container}>
      <h3>Performance Monitor</h3>
      
      <div className={styles.controls}>
        <label>
          Update Interval:
          <select
            value={updateInterval}
            onChange={(e) => setUpdateInterval(Number(e.target.value))}
          >
            <option value={500}>0.5s</option>
            <option value={1000}>1s</option>
            <option value={2000}>2s</option>
            <option value={5000}>5s</option>
          </select>
        </label>
      </div>

      <div className={styles.statsGrid}>
        <div className={styles.statCard}>
          <h4>Calculation Time</h4>
          <div className={styles.statValue}>
            <span>Avg: {stats.avgCalculationTime.toFixed(2)}ms</span>
            <span>Max: {stats.maxCalculationTime.toFixed(2)}ms</span>
          </div>
        </div>

        <div className={styles.statCard}>
          <h4>Memory Usage</h4>
          <div className={styles.statValue}>
            <span>Avg: {stats.avgMemoryUsage.toFixed(2)}KB</span>
            <span>Max: {stats.maxMemoryUsage.toFixed(2)}KB</span>
          </div>
        </div>

        <div className={styles.statCard}>
          <h4>Trajectory Points</h4>
          <div className={styles.statValue}>
            <span>Avg: {stats.avgPointCount.toFixed(0)}</span>
            <span>Total Calcs: {stats.totalCalculations}</span>
          </div>
        </div>
      </div>

      {trends && (
        <div className={styles.trends}>
          {renderTrendGraph(trends.calculationTimesTrend, 'Calculation Time Trend (ms)')}
          {renderTrendGraph(trends.memoryUsageTrend, 'Memory Usage Trend (KB)')}
          {renderTrendGraph(trends.pointCountTrend, 'Point Count Trend')}
        </div>
      )}

      <div className={styles.footer}>
        <button
          onClick={() => performanceMonitor.clearMetrics()}
          className={styles.clearButton}
        >
          Clear Metrics
        </button>
      </div>
    </div>
  );
};

export default PerformanceMonitor;
