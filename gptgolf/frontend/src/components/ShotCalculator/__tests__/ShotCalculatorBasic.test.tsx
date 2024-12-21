import React from 'react';
import { render, screen } from '@testing-library/react';
import '@testing-library/jest-dom';
import ShotCalculator from '../index';

describe('ShotCalculator Basic Tests', () => {
    test('renders club selector', () => {
        render(<ShotCalculator />);
        const clubSelector = screen.getByTestId('club-selector');
        expect(clubSelector).toBeInTheDocument();
    });

    test('renders weather info', () => {
        render(<ShotCalculator />);
        const weatherInfo = screen.getByTestId('weather-info');
        expect(weatherInfo).toBeInTheDocument();
    });

    test('renders trajectory view', () => {
        render(<ShotCalculator />);
        const trajectoryView = screen.getByTestId('trajectory-view');
        expect(trajectoryView).toBeInTheDocument();
    });
});
