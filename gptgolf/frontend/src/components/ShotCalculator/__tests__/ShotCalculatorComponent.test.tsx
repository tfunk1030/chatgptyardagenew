import React from 'react';
import { render, screen } from '@testing-library/react';
import '@testing-library/jest-dom';
import ShotCalculator from '../index';

describe('ShotCalculator', () => {
    test('renders club options', () => {
        render(<ShotCalculator />);
        
        // Test specific club options
        expect(screen.getByRole('option', { name: 'driver' })).toBeInTheDocument();
        expect(screen.getByRole('option', { name: '3-wood' })).toBeInTheDocument();
        expect(screen.getByRole('option', { name: '5-iron' })).toBeInTheDocument();
        expect(screen.getByRole('option', { name: '7-iron' })).toBeInTheDocument();
        expect(screen.getByRole('option', { name: 'pw' })).toBeInTheDocument();
    });

    test('renders weather inputs', () => {
        render(<ShotCalculator />);
        
        // Test weather inputs
        expect(screen.getByLabelText(/temperature/i)).toBeInTheDocument();
        expect(screen.getByLabelText(/wind speed/i)).toBeInTheDocument();
        expect(screen.getByLabelText(/wind direction/i)).toBeInTheDocument();
        expect(screen.getByLabelText(/humidity/i)).toBeInTheDocument();
        expect(screen.getByLabelText(/pressure/i)).toBeInTheDocument();
        expect(screen.getByLabelText(/altitude/i)).toBeInTheDocument();
    });

    test('renders trajectory view', () => {
        render(<ShotCalculator />);
        
        // Test trajectory view
        const trajectoryView = screen.getByTestId('trajectory-view');
        expect(trajectoryView).toBeInTheDocument();
        expect(trajectoryView.tagName.toLowerCase()).toBe('svg');
    });
});
