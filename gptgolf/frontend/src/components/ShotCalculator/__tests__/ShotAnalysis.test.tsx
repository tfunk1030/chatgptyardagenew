import React from 'react';
import { render, screen } from '@testing-library/react';
import ShotAnalysis from '../ShotAnalysis';

const mockShotData = {
  distance: 250,
  height: 100,
  spinRate: 2500,
  launchAngle: 12.0,
  ballSpeed: 150,
  flightTime: 6.2
};

const mockWindData = {
  speed: 10,
  direction: 45,
  altitude: 100,
  temperature: 20
};

const mockPreviousShots = [
  {
    distance: 245,
    height: 95,
    spinRate: 2400,
    launchAngle: 11.5,
    ballSpeed: 148,
    flightTime: 6.0
  },
  {
    distance: 255,
    height: 105,
    spinRate: 2600,
    launchAngle: 12.5,
    ballSpeed: 152,
    flightTime: 6.4
  }
];

describe('ShotAnalysis Component', () => {
  it('renders shot metrics correctly', () => {
    render(<ShotAnalysis shotData={mockShotData} />);
    
    expect(screen.getByText('Shot Analysis')).toBeInTheDocument();
    expect(screen.getByText('150.0 mph')).toBeInTheDocument();
    expect(screen.getByText('12.0°')).toBeInTheDocument();
    expect(screen.getByText('2500 rpm')).toBeInTheDocument();
  });

  it('calculates efficiency ratings correctly', () => {
    render(<ShotAnalysis shotData={mockShotData} />);
    
    // Perfect launch angle (12.0°) should show 100% efficiency
    expect(screen.getByText('100.0%')).toBeInTheDocument();
    
    // Perfect spin rate (2500 rpm) should show 100% efficiency
    const efficiencyValues = screen.getAllByText('100.0%');
    expect(efficiencyValues).toHaveLength(2);
  });

  it('shows wind impact when wind data is provided', () => {
    render(<ShotAnalysis shotData={mockShotData} windData={mockWindData} />);
    
    expect(screen.getByText(/Carry Effect/)).toBeInTheDocument();
    expect(screen.getByText(/Side Movement/)).toBeInTheDocument();
  });

  it('shows historical comparison when previous shots are provided', () => {
    render(
      <ShotAnalysis 
        shotData={mockShotData} 
        previousShots={mockPreviousShots}
      />
    );
    
    expect(screen.getByText(/Distance vs Avg/)).toBeInTheDocument();
    expect(screen.getByText(/Height vs Avg/)).toBeInTheDocument();
  });

  it('handles missing wind data gracefully', () => {
    render(<ShotAnalysis shotData={mockShotData} />);
    
    expect(screen.queryByText(/Wind Impact/)).not.toBeInTheDocument();
  });

  it('handles missing previous shots gracefully', () => {
    render(<ShotAnalysis shotData={mockShotData} />);
    
    expect(screen.queryByText(/Historical Comparison/)).not.toBeInTheDocument();
  });

  it('calculates wind effects accurately', () => {
    render(
      <ShotAnalysis 
        shotData={mockShotData} 
        windData={mockWindData}
      />
    );
    
    // With 10 m/s wind at 45°, expect significant carry and lateral effects
    const carryEffect = screen.getByText(/yards/, { selector: '.value' });
    const sideMovement = screen.getByText(/(Left|Right).*yards/, { selector: '.value' });
    
    expect(carryEffect).toBeInTheDocument();
    expect(sideMovement).toBeInTheDocument();
  });

  it('shows correct historical comparisons', () => {
    render(
      <ShotAnalysis 
        shotData={mockShotData} 
        previousShots={mockPreviousShots}
      />
    );
    
    // Average of previous shots: distance = 250, height = 100
    // Current shot: distance = 250, height = 100
    // Expect no difference
    expect(screen.getByText('0.0 yards')).toBeInTheDocument();
    expect(screen.getByText('0.0 feet')).toBeInTheDocument();
  });
});
