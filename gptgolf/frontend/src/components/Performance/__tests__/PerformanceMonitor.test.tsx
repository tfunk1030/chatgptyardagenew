import React from 'react';
import { render, screen, fireEvent, act } from '@testing-library/react';
import PerformanceMonitor from '../PerformanceMonitor';
import { performanceMonitor } from '../../../utils/shot-performance-monitor';

// Mock the performance monitor
jest.mock('../../../utils/shot-performance-monitor', () => ({
  performanceMonitor: {
    getStats: jest.fn(),
    getPerformanceTrends: jest.fn(),
    clearMetrics: jest.fn(),
  },
}));

describe('PerformanceMonitor Component', () => {
  const mockStats = {
    avgCalculationTime: 2.5,
    maxCalculationTime: 5.0,
    avgMemoryUsage: 1024,
    maxMemoryUsage: 2048,
    avgPointCount: 100,
    totalCalculations: 50,
  };

  const mockTrends = {
    calculationTimesTrend: [1.0, 1.5, 2.0, 2.5, 3.0],
    memoryUsageTrend: [900, 950, 1000, 1050, 1100],
    pointCountTrend: [90, 95, 100, 105, 110],
  };

  beforeEach(() => {
    jest.useFakeTimers();
    (performanceMonitor.getStats as jest.Mock).mockReturnValue(mockStats);
    (performanceMonitor.getPerformanceTrends as jest.Mock).mockReturnValue(mockTrends);
  });

  afterEach(() => {
    jest.clearAllMocks();
    jest.useRealTimers();
  });

  it('renders initial performance data', () => {
    render(<PerformanceMonitor />);

    // Check if stats are displayed
    expect(screen.getByText(/Calculation Time/)).toBeInTheDocument();
    expect(screen.getByText(/2.50ms/)).toBeInTheDocument();
    expect(screen.getByText(/Memory Usage/)).toBeInTheDocument();
    expect(screen.getByText(/1024.00KB/)).toBeInTheDocument();
    expect(screen.getByText(/Trajectory Points/)).toBeInTheDocument();
    expect(screen.getByText(/100/)).toBeInTheDocument();
  });

  it('updates data at specified intervals', () => {
    render(<PerformanceMonitor />);

    // Update mock values
    const newStats = { ...mockStats, avgCalculationTime: 3.0 };
    (performanceMonitor.getStats as jest.Mock).mockReturnValue(newStats);

    // Fast-forward time
    act(() => {
      jest.advanceTimersByTime(1000);
    });

    // Check if new values are displayed
    expect(screen.getByText(/3.00ms/)).toBeInTheDocument();
  });

  it('changes update interval when selected', () => {
    render(<PerformanceMonitor />);

    // Find and change the interval select
    const select = screen.getByRole('combobox');
    fireEvent.change(select, { target: { value: '2000' } });

    // Verify that old interval is cleared and new one is set
    expect(performanceMonitor.getStats).toHaveBeenCalledTimes(1);
    
    // Fast-forward time less than new interval
    act(() => {
      jest.advanceTimersByTime(1000);
    });
    expect(performanceMonitor.getStats).toHaveBeenCalledTimes(1);

    // Fast-forward time to complete new interval
    act(() => {
      jest.advanceTimersByTime(1000);
    });
    expect(performanceMonitor.getStats).toHaveBeenCalledTimes(2);
  });

  it('clears metrics when clear button is clicked', () => {
    render(<PerformanceMonitor />);

    // Find and click the clear button
    const clearButton = screen.getByText(/Clear Metrics/);
    fireEvent.click(clearButton);

    // Verify clearMetrics was called
    expect(performanceMonitor.clearMetrics).toHaveBeenCalledTimes(1);
  });

  it('renders trend graphs', () => {
    render(<PerformanceMonitor />);

    // Check if trend graphs are rendered
    expect(screen.getByText(/Calculation Time Trend/)).toBeInTheDocument();
    expect(screen.getByText(/Memory Usage Trend/)).toBeInTheDocument();
    expect(screen.getByText(/Point Count Trend/)).toBeInTheDocument();

    // Verify SVG elements are present
    const graphs = document.querySelectorAll('svg');
    expect(graphs.length).toBe(3); // One for each trend

    // Verify trend lines are rendered
    const trendLines = document.querySelectorAll('polyline');
    expect(trendLines.length).toBe(3);
  });

  it('handles missing trend data gracefully', () => {
    (performanceMonitor.getPerformanceTrends as jest.Mock).mockReturnValue({
      calculationTimesTrend: [],
      memoryUsageTrend: [],
      pointCountTrend: [],
    });

    render(<PerformanceMonitor />);

    // Should still render headers but no trend lines
    expect(screen.getByText(/Calculation Time Trend/)).toBeInTheDocument();
    const trendLines = document.querySelectorAll('polyline');
    expect(trendLines.length).toBe(0);
  });

  it('displays min/max values for trends', () => {
    render(<PerformanceMonitor />);

    // Check if min/max values are displayed for each trend
    expect(screen.getByText('Min: 1.00')).toBeInTheDocument();
    expect(screen.getByText('Max: 3.00')).toBeInTheDocument();
    expect(screen.getByText('Min: 900.00')).toBeInTheDocument();
    expect(screen.getByText('Max: 1100.00')).toBeInTheDocument();
  });

  it('unmounts cleanly', () => {
    const { unmount } = render(<PerformanceMonitor />);
    
    unmount();
    
    // Fast-forward time
    act(() => {
      jest.advanceTimersByTime(1000);
    });

    // Verify no more updates are attempted
    expect(performanceMonitor.getStats).toHaveBeenCalledTimes(1);
  });
});
