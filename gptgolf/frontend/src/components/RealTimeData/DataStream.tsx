import React, { useState } from 'react';
import { useLaunchMonitor, LaunchData } from '../../hooks/useLaunchMonitor';
import useShotPrediction from '../../hooks/useShotPrediction';
import ShotVisualizer from '../ShotVisualizer/ShotVisualizer';
import styles from './DataStream.module.css';

interface DataStreamProps {
    onShotData?: (data: LaunchData) => void;
    onPredictionComplete?: (prediction: any) => void;
}

interface ShotParams {
    clubSpeed: number;
    ballSpeed: number;
    launchAngle: number;
    spinRate: number;
    clubPath: number;
    faceAngle: number;
    attackAngle: number;
    dynamicLoft: number;
}

const defaultParams: ShotParams = {
    clubSpeed: 95,
    ballSpeed: 140,
    launchAngle: 12,
    spinRate: 2700,
    clubPath: 0,
    faceAngle: 0,
    attackAngle: -4,
    dynamicLoft: 11
};

const tooltips = {
    clubSpeed: 'Speed of the club head at impact (60-130 mph)',
    ballSpeed: 'Speed of the ball after impact (90-200 mph)',
    launchAngle: 'Initial angle of the ball relative to the ground (0-40°)',
    spinRate: 'Rate of backspin on the ball (1000-10000 rpm)'
};

const DataStream: React.FC<DataStreamProps> = ({
    onShotData,
    onPredictionComplete
}) => {
    const [shotHistory, setShotHistory] = useState<LaunchData[]>([]);
    const [currentTrajectory, setCurrentTrajectory] = useState<any>(null);
    const [isProcessing, setIsProcessing] = useState(false);
    const [params, setParams] = useState<ShotParams>(defaultParams);

    const { predictShot, isLoading: isPredicting } = useShotPrediction();
    const { error } = useLaunchMonitor();

    const handleParamChange = (field: keyof ShotParams, value: string) => {
        const numValue = parseFloat(value);
        if (!isNaN(numValue)) {
            setParams(prev => ({
                ...prev,
                [field]: numValue
            }));
        }
    };

    const handleTakeShot = async () => {
        const shotData: LaunchData = {
            ...params,
            timestamp: Date.now()
        };

        setShotHistory(prev => [...prev, shotData]);
        onShotData?.(shotData);
        
        setIsProcessing(true);
        try {
            const prediction = await predictShot({
                ...shotData,
                weather: {
                    temperature: 72,
                    humidity: 50,
                    pressure: 1013.25,
                    windSpeed: 0,
                    windDirection: 0
                }
            });

            if (prediction) {
                setCurrentTrajectory(prediction.trajectory);
                onPredictionComplete?.(prediction);
            }
        } catch (err) {
            console.error('Prediction error:', err);
        } finally {
            setIsProcessing(false);
        }
    };

    return (
        <div className={styles.container}>
            <div className={styles.controls}>
                <div className={styles.inputGrid}>
                    <div className={styles.inputGroup}>
                        <label htmlFor="clubSpeed">Club Speed (mph)</label>
                        <input
                            id="clubSpeed"
                            type="number"
                            value={params.clubSpeed}
                            onChange={(e) => handleParamChange('clubSpeed', e.target.value)}
                            min="60"
                            max="130"
                            step="0.1"
                            title={tooltips.clubSpeed}
                        />
                        <div className={styles.tooltip}>{tooltips.clubSpeed}</div>
                    </div>
                    <div className={styles.inputGroup}>
                        <label htmlFor="ballSpeed">Ball Speed (mph)</label>
                        <input
                            id="ballSpeed"
                            type="number"
                            value={params.ballSpeed}
                            onChange={(e) => handleParamChange('ballSpeed', e.target.value)}
                            min="90"
                            max="200"
                            step="0.1"
                            title={tooltips.ballSpeed}
                        />
                        <div className={styles.tooltip}>{tooltips.ballSpeed}</div>
                    </div>
                    <div className={styles.inputGroup}>
                        <label htmlFor="launchAngle">Launch Angle (°)</label>
                        <input
                            id="launchAngle"
                            type="number"
                            value={params.launchAngle}
                            onChange={(e) => handleParamChange('launchAngle', e.target.value)}
                            min="0"
                            max="40"
                            step="0.1"
                            title={tooltips.launchAngle}
                        />
                        <div className={styles.tooltip}>{tooltips.launchAngle}</div>
                    </div>
                    <div className={styles.inputGroup}>
                        <label htmlFor="spinRate">Spin Rate (rpm)</label>
                        <input
                            id="spinRate"
                            type="number"
                            value={params.spinRate}
                            onChange={(e) => handleParamChange('spinRate', e.target.value)}
                            min="1000"
                            max="10000"
                            step="100"
                            title={tooltips.spinRate}
                        />
                        <div className={styles.tooltip}>{tooltips.spinRate}</div>
                    </div>
                </div>

                <button 
                    onClick={handleTakeShot}
                    disabled={isProcessing}
                    className={styles.simulateButton}
                >
                    Calculate Shot
                </button>
                
                <div className={styles.processingStatus}>
                    {isProcessing && <div className={styles.processing}>Processing shot data...</div>}
                    {isPredicting && <div className={styles.predicting}>Calculating trajectory...</div>}
                </div>
                
                {error && !error.message.includes('disabled') && (
                    <small className={styles.error}>{error.message}</small>
                )}
            </div>

            {currentTrajectory && (
                <div className={styles.visualizer}>
                    <ShotVisualizer
                        trajectory={currentTrajectory}
                        showGrid={true}
                        showAxes={true}
                        width={800}
                        height={600}
                    />
                </div>
            )}

            <div className={styles.history}>
                <h3>Shot History</h3>
                <div className={styles.shotList}>
                    {shotHistory.slice(-5).reverse().map((shot, index) => (
                        <div key={shot.timestamp} className={styles.shotItem}>
                            <div className={styles.shotTime}>
                                {new Date(shot.timestamp).toLocaleTimeString()}
                            </div>
                            <div className={styles.shotDetails}>
                                <div>Club Speed: {shot.clubSpeed.toFixed(1)} mph</div>
                                <div>Ball Speed: {shot.ballSpeed.toFixed(1)} mph</div>
                                <div>Launch Angle: {shot.launchAngle.toFixed(1)}°</div>
                                <div>Spin Rate: {shot.spinRate.toFixed(0)} rpm</div>
                            </div>
                        </div>
                    ))}
                </div>
            </div>
        </div>
    );
};

export default DataStream;
