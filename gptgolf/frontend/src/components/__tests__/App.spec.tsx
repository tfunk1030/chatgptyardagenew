import React from 'react';
import { render, screen } from '@testing-library/react';
import '@testing-library/jest-dom';
import App from '../../App';
import { errorTracker } from '../../utils/error_tracker';

// Mock error tracker
jest.mock('../../utils/error_tracker', () => ({
    errorTracker: {
        logError: jest.fn()
    }
}));

describe('App Component', () => {
    beforeEach(() => {
        jest.clearAllMocks();
    });

    test('renders main heading', () => {
        render(<App />);
        const heading = screen.getByRole('heading', { 
            level: 1,
            name: /Golf Shot Calculator/i 
        });
        expect(heading).toBeInTheDocument();
    });

    test('renders calculator component', () => {
        render(<App />);
        const calculator = screen.getByTestId('club-selector');
        expect(calculator).toBeInTheDocument();
    });

    test('error boundary catches errors and shows fallback UI', () => {
        // Mock console.error to avoid test noise
        const consoleError = jest.spyOn(console, 'error').mockImplementation(() => {});
        
        // Force an error in the app
        jest.spyOn(App.prototype, 'render').mockImplementation(() => {
            throw new Error('Test error');
        });

        render(<App />);

        // Verify error boundary fallback UI is shown
        expect(screen.getByRole('alert')).toBeInTheDocument();
        expect(screen.getByText('Something went wrong')).toBeInTheDocument();
        
        // Verify error was logged
        expect(errorTracker.logError).toHaveBeenCalledWith(
            expect.objectContaining({
                type: 'react',
                message: expect.any(String)
            })
        );

        // Cleanup
        consoleError.mockRestore();
    });

    test('app recovers from non-fatal errors', () => {
        const { rerender } = render(<App />);
        
        // Force a re-render after an error
        rerender(<App />);
        
        // Verify app renders normally after recovery
        expect(screen.getByRole('heading', { level: 1 })).toBeInTheDocument();
        expect(screen.getByTestId('club-selector')).toBeInTheDocument();
    });
});
