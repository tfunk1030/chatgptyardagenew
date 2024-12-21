import { renderHook, act } from '@testing-library/react-hooks';
import * as tf from '@tensorflow/tfjs';
import useShotPrediction from '../useShotPrediction';

// Mock TensorFlow.js
jest.mock('@tensorflow/tfjs', () => ({
    loadLayersModel: jest.fn(),
    tensor2d: jest.fn(),
    ones: jest.fn(),
    dispose: jest.fn()
}));

describe('useShotPrediction', () => {
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

    beforeEach(() => {
        // Setup TensorFlow mocks
        (tf.loadLayersModel as jest.Mock).mockResolvedValue({
            predict: jest.fn().mockReturnValue({
                data: jest.fn().mockResolvedValue(mockPrediction),
                dispose: jest.fn()
            }),
            dispose: jest.fn()
        });

        (tf.tensor2d as jest.Mock).mockReturnValue({
            concat: jest.fn().mockReturnThis(),
            dispose: jest.fn()
        });

        (tf.ones as jest.Mock).mockReturnValue({
            dispose: jest.fn()
        });
    });

    afterEach(() => {
        jest.clearAllMocks();
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
        (tf.loadLayersModel as jest.Mock).mockRejectedValue(new Error('Failed to load model'));
        
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
        
        // Predictions should be different with weather conditions
        if (predictionWithWeather && predictionWithoutWeather) {
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
});
