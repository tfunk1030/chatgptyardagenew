import React from 'react';
import { render, screen } from '@testing-library/react';
import '@testing-library/jest-dom';
import ShotCalculator from '../index';

test('renders club selector', () => {
    render(<ShotCalculator />);
    const clubSelector = screen.queryByTestId('club-selector');
    expect(clubSelector).toBeTruthy();
});

test('renders weather info', () => {
    render(<ShotCalculator />);
    const weatherInfo = screen.queryByTestId('weather-info');
    expect(weatherInfo).toBeTruthy();
});

test('renders trajectory view', () => {
    render(<ShotCalculator />);
    const trajectoryView = screen.queryByTestId('trajectory-view');
    expect(trajectoryView).toBeTruthy();
});
