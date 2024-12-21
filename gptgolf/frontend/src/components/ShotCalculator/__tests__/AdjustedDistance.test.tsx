import React from 'react';
import { render, screen } from '@testing-library/react';
import AdjustedDistance from '../AdjustedDistance';

describe('AdjustedDistance', () => {
  const defaultProps = {
    actualDistance: 150,
    unit: 'yards' as const,
    weather: {
      temperature: 72,
      humidity: 50,
      pressure: 29.92,
      windSpeed: 0,
      windDirection: 0,
      altitude: 0
    }
  };

  it('renders without crashing', () => {
    render(<AdjustedDistance {...defaultProps} />);
    expect(screen.getByText('Actual Distance')).toBeInTheDocument();
    expect(screen.getByText('Plays Like')).toBeInTheDocument();
  });

  describe('Wind Effect Calculations', () => {
    it('handles headwind correctly', () => {
      const props = {
        ...defaultProps,
        weather: {
          ...defaultProps.weather,
          windSpeed: 10,
          windDirection: 0  // Direct headwind
        }
      };
      render(<AdjustedDistance {...props} />);
      // 10mph headwind with 0.35 coefficient should add ~3.5% to distance
      expect(screen.getByText(/plays.*3.5%.*longer/i)).toBeInTheDocument();
    });

    it('handles tailwind correctly', () => {
      const props = {
        ...defaultProps,
        weather: {
          ...defaultProps.weather,
          windSpeed: 10,
          windDirection: 180  // Direct tailwind
        }
      };
      render(<AdjustedDistance {...props} />);
      // 10mph tailwind with 0.35 coefficient should reduce ~3.5% from distance
      expect(screen.getByText(/plays.*3.5%.*shorter/i)).toBeInTheDocument();
    });

    it('handles crosswind correctly', () => {
      const props = {
        ...defaultProps,
        weather: {
          ...defaultProps.weather,
          windSpeed: 10,
          windDirection: 90  // Direct crosswind
        }
      };
      render(<AdjustedDistance {...props} />);
      // 10mph crosswind with 0.15 coefficient should reduce ~1.5% from distance
      expect(screen.getByText(/plays.*1.5%.*shorter/i)).toBeInTheDocument();
    });

    it('applies shot type multipliers correctly', () => {
      const baseProps = {
        ...defaultProps,
        weather: {
          ...defaultProps.weather,
          windSpeed: 10,
          windDirection: 0  // Headwind
        }
      };

      // Low shot (0.7 multiplier)
      const { rerender } = render(<AdjustedDistance {...baseProps} shotType="low" />);
      expect(screen.getByText(/plays.*2.4%.*longer/i)).toBeInTheDocument();

      // High shot (1.4 multiplier)
      rerender(<AdjustedDistance {...baseProps} shotType="high" />);
      expect(screen.getByText(/plays.*4.9%.*longer/i)).toBeInTheDocument();
    });

    it('applies terrain factors correctly', () => {
      const baseProps = {
        ...defaultProps,
        weather: {
          ...defaultProps.weather,
          windSpeed: 10,
          windDirection: 0  // Headwind
        }
      };

      // Water (1.2 multiplier)
      const { rerender } = render(<AdjustedDistance {...baseProps} terrain="water" />);
      expect(screen.getByText(/plays.*4.2%.*longer/i)).toBeInTheDocument();

      // Rough (0.8 multiplier)
      rerender(<AdjustedDistance {...baseProps} terrain="rough" />);
      expect(screen.getByText(/plays.*2.8%.*longer/i)).toBeInTheDocument();
    });

    it('handles strong wind non-linear scaling', () => {
      const props = {
        ...defaultProps,
        weather: {
          ...defaultProps.weather,
          windSpeed: 20,  // Strong wind
          windDirection: 0  // Headwind
        }
      };
      render(<AdjustedDistance {...props} />);
      // Should include additional scaling for wind > 15mph
      expect(screen.getByText(/plays.*7.7%.*longer/i)).toBeInTheDocument();
    });
  });

  describe('Other Weather Effects', () => {
    it('calculates temperature effects correctly', () => {
      const props = {
        ...defaultProps,
        weather: {
          ...defaultProps.weather,
          temperature: 82  // 10°F above standard
        }
      };
      render(<AdjustedDistance {...props} />);
      // 10°F above standard should reduce distance by 1.5%
      expect(screen.getByText(/82°F:.*1.5%.*shorter/i)).toBeInTheDocument();
    });

    it('calculates pressure effects correctly', () => {
      const props = {
        ...defaultProps,
        weather: {
          ...defaultProps.weather,
          pressure: 30.92  // 1 inHg above standard
        }
      };
      render(<AdjustedDistance {...props} />);
      // 1 inHg above standard should reduce distance by 2.5%
      expect(screen.getByText(/30.92 inHg:.*2.5%.*shorter/i)).toBeInTheDocument();
    });

    it('calculates altitude effects correctly', () => {
      const props = {
        ...defaultProps,
        weather: {
          ...defaultProps.weather,
          altitude: 1000  // 1000ft elevation
        }
      };
      render(<AdjustedDistance {...props} />);
      // 1000ft should reduce by 2%
      expect(screen.getByText(/1000ft elevation:.*2%.*shorter/i)).toBeInTheDocument();
    });

    it('calculates humidity effects correctly', () => {
      const props = {
        ...defaultProps,
        weather: {
          ...defaultProps.weather,
          humidity: 70  // 20% above standard
        }
      };
      render(<AdjustedDistance {...props} />);
      // 20% above standard should reduce distance by 1%
      expect(screen.getByText(/70% humidity:.*1%.*shorter/i)).toBeInTheDocument();
    });
  });

  it('combines all effects correctly', () => {
    const props = {
      ...defaultProps,
      weather: {
        temperature: 82,    // +1.5% shorter
        humidity: 70,       // +1% shorter
        pressure: 30.92,    // +2.5% shorter
        windSpeed: 10,      // ~3.5% longer (headwind)
        windDirection: 0,
        altitude: 1000      // +2% shorter
      }
    };
    render(<AdjustedDistance {...props} />);
    // Net effect should be approximately 3.5% shorter
    expect(screen.getByText(/Total Adjustment/)).toBeInTheDocument();
    expect(screen.getByText(/Plays.*3.5%.*shorter/i)).toBeInTheDocument();
  });
});
