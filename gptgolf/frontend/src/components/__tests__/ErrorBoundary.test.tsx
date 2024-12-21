import React from 'react';
import { render, screen } from '@testing-library/react';
import { ErrorBoundary } from '../ErrorBoundary';
import { errorTracker } from '../../utils/error_tracker';

// Mock error tracker
jest.mock('../../utils/error_tracker', () => ({
    errorTracker: {
        logError: jest.fn()
    }
}));

// Test component that throws an error
const ThrowError = ({ message }: { message: string }) => {
    throw new Error(message);
};

describe('ErrorBoundary', () => {
    // Clear mocks before each test
    beforeEach(() => {
        jest.clearAllMocks();
        jest.spyOn(console, 'error').mockImplementation(() => {});
    });

    afterEach(() => {
        jest.restoreAllMocks();
    });

    it('renders children when there is no error', () => {
        const { getByText } = render(
            <ErrorBoundary>
                <div>Test Content</div>
            </ErrorBoundary>
        );
        
        expect(getByText('Test Content')).toBeInTheDocument();
    });

    it('renders fallback UI when there is an error', () => {
        const errorMessage = 'Test error';
        
        render(
            <ErrorBoundary>
                <ThrowError message={errorMessage} />
            </ErrorBoundary>
        );

        expect(screen.getByRole('alert')).toBeInTheDocument();
        expect(screen.getByText('Something went wrong')).toBeInTheDocument();
        expect(screen.getByText('Please try refreshing the page')).toBeInTheDocument();
    });

    it('renders custom fallback when provided', () => {
        const customFallback = <div>Custom Error Message</div>;
        
        render(
            <ErrorBoundary fallback={customFallback}>
                <ThrowError message="Test error" />
            </ErrorBoundary>
        );

        expect(screen.getByText('Custom Error Message')).toBeInTheDocument();
    });

    it('calls error tracker when error occurs', () => {
        const errorMessage = 'Test error';
        
        render(
            <ErrorBoundary>
                <ThrowError message={errorMessage} />
            </ErrorBoundary>
        );

        expect(errorTracker.logError).toHaveBeenCalledWith(
            expect.objectContaining({
                type: 'react',
                message: errorMessage,
                metadata: expect.any(Object)
            })
        );
    });

    it('calls onError prop when error occurs', () => {
        const onError = jest.fn();
        const errorMessage = 'Test error';
        
        render(
            <ErrorBoundary onError={onError}>
                <ThrowError message={errorMessage} />
            </ErrorBoundary>
        );

        expect(onError).toHaveBeenCalledWith(
            expect.any(Error),
            expect.objectContaining({
                componentStack: expect.any(String)
            })
        );
    });
});
