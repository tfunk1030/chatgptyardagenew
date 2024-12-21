import { useState, useEffect, useCallback } from 'react';
import * as tf from '@tensorflow/tfjs';

interface ShotInputs {
    clubSpeed: number;
    clubPath: number;
    faceAngle: number;
    attackAngle: number;
    dynamicLoft: number;
    spinRate: number;
    launchAngle: number;
    ballSpeed: number;
    weather?: {
        temperature: number;
        humidity: number;
        pressure: number;
        windSpeed: number;
        windDirection: number;
    };
}

interface PredictedShot {
    trajectory: Array<{
        position: { x: number; y: number; z: number };
        time: number;
        velocity: { x: number; y: number; z: number };
        spin: { rate: number; axis: number };
    }>;
    carry: number;
    totalDistance: number;
    apex: number;
    spinRate: number;
    landingAngle: number;
    confidence: number;
}

interface ModelState {
    isLoading: boolean;
    error: string | null;
    model: tf.LayersModel | null;
}

const useShotPrediction = () => {
    const [modelState, setModelState] = useState<ModelState>({
        isLoading: true,
        error: null,
        model: null
    });

    // Load and initialize the model
    useEffect(() => {
        const loadModel = async () => {
            try {
                // Load the model architecture and weights
                const model = await tf.loadLayersModel('/models/shot-prediction/model.json');
                
                // Warm up the model with a sample prediction
                const warmupTensor = tf.ones([1, 8]); // 8 input features
                model.predict(warmupTensor);
                warmupTensor.dispose();

                setModelState({
                    isLoading: false,
                    error: null,
                    model
                });
            } catch (err) {
                setModelState({
                    isLoading: false,
                    error: 'Failed to load shot prediction model',
                    model: null
                });
            }
        };

        loadModel();

        // Cleanup
        return () => {
            if (modelState.model) {
                modelState.model.dispose();
            }
        };
    }, []);

    // Normalize input features
    const normalizeInputs = useCallback((inputs: ShotInputs) => {
        // Feature scaling parameters (pre-computed from training data)
        const SCALING = {
            clubSpeed: { mean: 40.0, std: 10.0 },
            clubPath: { mean: 0.0, std: 5.0 },
            faceAngle: { mean: 0.0, std: 5.0 },
            attackAngle: { mean: -4.0, std: 4.0 },
            dynamicLoft: { mean: 14.0, std: 5.0 },
            spinRate: { mean: 3000, std: 1000 },
            launchAngle: { mean: 12.0, std: 5.0 },
            ballSpeed: { mean: 60.0, std: 15.0 }
        };

        return [
            (inputs.clubSpeed - SCALING.clubSpeed.mean) / SCALING.clubSpeed.std,
            (inputs.clubPath - SCALING.clubPath.mean) / SCALING.clubPath.std,
            (inputs.faceAngle - SCALING.faceAngle.mean) / SCALING.faceAngle.std,
            (inputs.attackAngle - SCALING.attackAngle.mean) / SCALING.attackAngle.std,
            (inputs.dynamicLoft - SCALING.dynamicLoft.mean) / SCALING.dynamicLoft.std,
            (inputs.spinRate - SCALING.spinRate.mean) / SCALING.spinRate.std,
            (inputs.launchAngle - SCALING.launchAngle.mean) / SCALING.launchAngle.std,
            (inputs.ballSpeed - SCALING.ballSpeed.mean) / SCALING.ballSpeed.std
        ];
    }, []);

    // Generate trajectory points from model output
    const generateTrajectory = useCallback((prediction: Float32Array) => {
        const NUM_POINTS = 50;
        const trajectory = [];
        let currentTime = 0;
        const timeStep = prediction[0] / NUM_POINTS; // Total flight time

        for (let i = 0; i < NUM_POINTS; i++) {
            const t = i / (NUM_POINTS - 1);
            
            // Extract predicted parameters
            const maxHeight = prediction[1];
            const totalDistance = prediction[2];
            const spinDecayRate = prediction[3];
            const initialSpinRate = prediction[4];
            const launchDirection = prediction[5];
            
            // Calculate position using ballistic equations with spin effects
            const x = totalDistance * t;
            const y = maxHeight * Math.sin(Math.PI * t);
            const z = launchDirection * x * (1 - t); // Curve due to side spin
            
            // Calculate velocities (derivatives of position)
            const vx = totalDistance / prediction[0]; // Constant x velocity
            const vy = maxHeight * Math.PI * Math.cos(Math.PI * t) / prediction[0];
            const vz = launchDirection * (1 - 2 * t) * vx;
            
            // Calculate spin rate with decay
            const spinRate = initialSpinRate * Math.exp(-spinDecayRate * t);
            
            trajectory.push({
                position: { x, y, z },
                time: currentTime,
                velocity: { x: vx, y: vy, z: vz },
                spin: {
                    rate: spinRate,
                    axis: launchDirection * Math.PI / 180 // Convert to radians
                }
            });
            
            currentTime += timeStep;
        }

        return trajectory;
    }, []);

    // Main prediction function
    const predictShot = useCallback(async (inputs: ShotInputs): Promise<PredictedShot | null> => {
        if (!modelState.model || modelState.isLoading) {
            return null;
        }

        try {
            // Prepare input tensor
            const normalizedInputs = normalizeInputs(inputs);
            const inputTensor = tf.tensor2d([normalizedInputs]);

            // Add weather conditions if available
            if (inputs.weather) {
                const weatherTensor = tf.tensor2d([[
                    inputs.weather.temperature / 30.0, // Normalize to [-1, 1]
                    inputs.weather.humidity / 100.0,
                    (inputs.weather.pressure - 1013.25) / 50.0,
                    inputs.weather.windSpeed / 20.0,
                    inputs.weather.windDirection / 180.0
                ]]);
                inputTensor.concat(weatherTensor, 1);
            }

            // Make prediction
            const predictionTensor = modelState.model.predict(inputTensor) as tf.Tensor;
            const prediction = await predictionTensor.data();

            // Clean up tensors
            inputTensor.dispose();
            predictionTensor.dispose();

            // Generate full trajectory
            const trajectory = generateTrajectory(prediction as Float32Array);

            return {
                trajectory,
                carry: prediction[2], // Total distance
                totalDistance: prediction[2] * 1.1, // Approximate with roll
                apex: prediction[1], // Max height
                spinRate: prediction[4], // Initial spin rate
                landingAngle: Math.atan2(trajectory[trajectory.length - 2].velocity.y,
                                       trajectory[trajectory.length - 2].velocity.x) * 180 / Math.PI,
                confidence: prediction[6] // Model confidence score
            };
        } catch (err) {
            console.error('Prediction error:', err);
            return null;
        }
    }, [modelState.model, modelState.isLoading, normalizeInputs, generateTrajectory]);

    return {
        predictShot,
        isLoading: modelState.isLoading,
        error: modelState.error
    };
};

export default useShotPrediction;
