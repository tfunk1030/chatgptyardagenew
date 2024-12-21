import { renderHook, act } from '@testing-library/react-hooks';
import * as tf from '@tensorflow/tfjs';
import useShotPrediction from '../useShotPrediction';

// Mock data
const mockInputs = {
    clubSpeed: 45.0,
    clubPath: 2.0,
    faceAngle: 1.5,
    attackAngle: -4.5,
    dynamicLoft: 14.0,
    spinRate: 2800,
    launchAngle: 12.5,
    ballSpeed: 65.0,
    weather: {
        temperature: 20.0,
        humidity: 60.0,
        pressure: 1013.25,
        windSpeed: 5.0,
        windDirection: 90.0
    }
};

const mockPrediction = new Float32Array([
    3.5,    // flight time
    35.0,   // max height
    250.0,  // total distance
    0.2,    // spin decay rate
    2800.0, // initial spin rate
    2.0,    // launch direction
    0.95    // confidence
]);

// Mock TensorFlow.js
jest.mock('@tensorflow/tfjs', () => ({
    loadLayersModel: jest.fn(),
    tensor2d: jest.fn(),
    ones: jest.fn(),
    dispose: jest.fn(),
    ready: jest.fn().mockResolvedValue(true),
    browser: {
        fromPixels: jest.fn()
    },
    memory: jest.fn(() => ({
        numBytes: 0,
        numTensors: 0,
        numDataBuffers: 0,
        unreliable: false
    })),
    tidy: jest.fn((fn: () => any) => fn())
}));

describe('useShotPrediction', () => {
    beforeEach(() => {
        // Reset all mocks
        jest.clearAllMocks();

        // Mock TensorFlow.js model loading and prediction
        (tf.loadLayersModel as jest.Mock).mockResolvedValue({
            predict: jest.fn().mockReturnValue({
                data: jest.fn().mockResolvedValue(mockPrediction),
                dispose: jest.fn()
            }),
            dispose: jest.fn()
        });

        // Mock tensor creation
        (tf.tensor2d as jest.Mock).mockReturnValue({
            concat: jest.fn().mockReturnThis(),
            dispose: jest.fn()
        });
    });

    it('loads the model successfully', async () => {
        const { result, waitForNextUpdate } = renderHook(() => useShotPrediction());
        
        expect(result.current.isLoading).toBe(true);
        expect(result.current.error).toBeNull();
        
        await waitForNextUpdate();
        
        expect(result.current.isLoading).toBe(false);
        expect(result.current.error).toBeNull();
        expect(tf.loadLayersModel).toHaveBeenCalledWith('/models/shot-prediction/model.json');
    });

    it('handles model loading failure', async () => {
        const error = new Error('Failed to load model');
        (tf.loadLayersModel as jest.Mock).mockRejectedValue(error);
        
        const { result, waitForNextUpdate } = renderHook(() => useShotPrediction());
        
        await waitForNextUpdate();
        
        expect(result.current.isLoading).toBe(false);
        expect(result.current.error).toBe('Failed to load shot prediction model');
    });

    it('makes predictions successfully', async () => {
        const { result, waitForNextUpdate } = renderHook(() => useShotPrediction());
        
        await waitForNextUpdate();
        
        const prediction = await result.current.predictShot(mockInputs);
        
        expect(prediction).not.toBeNull();
        if (prediction) {
            expect(prediction.carry).toBe(250.0);
            expect(prediction.totalDistance).toBeGreaterThan(prediction.carry);
            expect(prediction.apex).toBe(35.0);
            expect(prediction.spinRate).toBe(2800.0);
            expect(prediction.confidence).toBe(0.95);
            expect(prediction.trajectory.length).toBeGreaterThan(0);
        }
    });

    it('generates correct trajectory points', async () => {
        const { result, waitForNextUpdate } = renderHook(() => useShotPrediction());
        
        await waitForNextUpdate();
        
        const prediction = await result.current.predictShot(mockInputs);
        
        expect(prediction).not.toBeNull();
        if (prediction) {
            const { trajectory } = prediction;
            
            // Check first point
            expect(trajectory[0].position.x).toBe(0);
            expect(trajectory[0].position.y).toBe(0);
            expect(trajectory[0].position.z).toBe(0);
            
            // Check trajectory shape
            const midPoint = Math.floor(trajectory.length / 2);
            expect(trajectory[midPoint].position.y).toBeGreaterThan(trajectory[0].position.y);
            expect(trajectory[midPoint].position.y).toBeGreaterThan(trajectory[trajectory.length - 1].position.y);
            
            // Check last point
            expect(trajectory[trajectory.length - 1].position.y).toBeCloseTo(0, 1);
            expect(trajectory[trajectory.length - 1].time).toBeCloseTo(mockPrediction[0], 2);
        }
    });

    it('handles weather conditions correctly', async () => {
        const { result, waitForNextUpdate } = renderHook(() => useShotPrediction());
        
        await waitForNextUpdate();
        
        // Make prediction with weather
        const predictionWithWeather = await result.current.predictShot(mockInputs);
        
        // Make prediction without weather
        const predictionWithoutWeather = await result.current.predictShot({
            ...mockInputs,
            weather: undefined
        });
        
        expect(predictionWithWeather).not.toBeNull();
        expect(predictionWithoutWeather).not.toBeNull();
        
        if (predictionWithWeather && predictionWithoutWeather) {
            // Predictions should be different with weather conditions
            expect(predictionWithWeather.carry).not.toBe(predictionWithoutWeather.carry);
            expect(predictionWithWeather.trajectory).not.toEqual(predictionWithoutWeather.trajectory);
        }
    });

    it('normalizes inputs correctly', async () => {
        const { result, waitForNextUpdate } = renderHook(() => useShotPrediction());
        
        await waitForNextUpdate();
        
        await act(async () => {
            await result.current.predictShot(mockInputs);
        });
        
        // Verify tensor2d was called with normalized values
        expect(tf.tensor2d).toHaveBeenCalledWith([expect.any(Array)]);
        const normalizedInputs = (tf.tensor2d as jest.Mock).mock.calls[0][0][0];
        
        // Check normalization (values should be roughly between -3 and 3 for standard scaling)
        normalizedInputs.forEach((value: number) => {
            expect(Math.abs(value)).toBeLessThan(3);
        });
    });

    it('handles spin decay correctly', async () => {
        const { result, waitForNextUpdate } = renderHook(() => useShotPrediction());
        
        await waitForNextUpdate();
        
        const prediction = await result.current.predictShot(mockInputs);
        
        expect(prediction).not.toBeNull();
        if (prediction) {
            const { trajectory } = prediction;
            
            // Verify spin rate decreases over time
            for (let i = 1; i < trajectory.length; i++) {
                expect(trajectory[i].spin.rate).toBeLessThan(trajectory[i-1].spin.rate);
            }
        }
    });

    it('cleans up resources on unmount', () => {
        const { unmount } = renderHook(() => useShotPrediction());
        
        unmount();
        
        // Verify model and tensor disposal
        expect(tf.dispose).toHaveBeenCalled();
    });

    it('handles invalid inputs gracefully', async () => {
        const { result, waitForNextUpdate } = renderHook(() => useShotPrediction());
        
        await waitForNextUpdate();
        
        // Test with invalid inputs
        const invalidInputs = {
            ...mockInputs,
            clubSpeed: -1, // Invalid negative speed
            spinRate: 15000 // Unrealistically high spin rate
        };
        
        const prediction = await result.current.predictShot(invalidInputs);
        
        // Should still return a prediction, but with lower confidence
        expect(prediction).not.toBeNull();
        if (prediction) {
            expect(prediction.confidence).toBeLessThan(0.8);
        }
    });

    it('applies weather effects realistically', async () => {
        const { result, waitForNextUpdate } = renderHook(() => useShotPrediction());
        
        await waitForNextUpdate();
        
        // Test with strong headwind
        const headwindInputs = {
            ...mockInputs,
            weather: {
                ...mockInputs.weather!,
                windSpeed: 10.0,
                windDirection: 180.0
            }
        };
        
        const headwindPrediction = await result.current.predictShot(headwindInputs);
        
        // Test with strong tailwind
        const tailwindInputs = {
            ...mockInputs,
            weather: {
                ...mockInputs.weather!,
                windSpeed: 10.0,
                windDirection: 0.0
            }
        };
        
        const tailwindPrediction = await result.current.predictShot(tailwindInputs);
        
        expect(headwindPrediction).not.toBeNull();
        expect(tailwindPrediction).not.toBeNull();
        
        if (headwindPrediction && tailwindPrediction) {
            // Headwind should result in shorter carry
            expect(headwindPrediction.carry).toBeLessThan(tailwindPrediction.carry);
            
            // Headwind should result in higher apex
            expect(headwindPrediction.apex).toBeGreaterThan(tailwindPrediction.apex);
        }
    });
});
