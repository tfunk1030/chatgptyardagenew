import { useState, useCallback } from 'react';

export interface LaunchData {
    clubSpeed: number;
    clubPath: number;
    faceAngle: number;
    attackAngle: number;
    dynamicLoft: number;
    spinRate: number;
    launchAngle: number;
    ballSpeed: number;
    timestamp: number;
}

interface UseLaunchMonitorProps {
    onData?: (data: LaunchData) => void;
    onError?: (error: Error) => void;
    enableRealDevice?: boolean; // New prop to explicitly enable real device connection
}

const WEBSOCKET_URL = 'ws://localhost:8080/launch-monitor';

export const useLaunchMonitor = ({ onData, onError, enableRealDevice = false }: UseLaunchMonitorProps = {}) => {
    const [isConnected, setIsConnected] = useState(false);
    const [lastData, setLastData] = useState<LaunchData | null>(null);
    const [error, setError] = useState<Error | null>(null);

    // Simulate realistic shot data based on club type and skill level
    const simulateShot = useCallback(() => {
        const mockData: LaunchData = {
            // Realistic driver shot data ranges
            clubSpeed: 95 + Math.random() * 15, // 95-110 mph
            clubPath: -2 + Math.random() * 4, // -2 to +2 degrees
            faceAngle: -1 + Math.random() * 2, // -1 to +1 degrees
            attackAngle: -2 + Math.random() * 4, // -2 to +2 degrees
            dynamicLoft: 11 + Math.random() * 4, // 11-15 degrees
            spinRate: 2500 + Math.random() * 1000, // 2500-3500 rpm
            launchAngle: 12 + Math.random() * 4, // 12-16 degrees
            ballSpeed: 145 + Math.random() * 20, // 145-165 mph
            timestamp: Date.now()
        };

        setLastData(mockData);
        onData?.(mockData);
    }, [onData]);

    // Connect to a real launch monitor only if explicitly enabled
    const connect = useCallback(() => {
        if (!enableRealDevice) {
            const error = new Error('Launch monitor connection is disabled. Enable it in settings to use a real device.');
            setError(error);
            onError?.(error);
            return;
        }

        try {
            const ws = new WebSocket(WEBSOCKET_URL);

            ws.onopen = () => {
                setIsConnected(true);
                setError(null);
            };

            ws.onmessage = (event) => {
                try {
                    const data: LaunchData = JSON.parse(event.data);
                    setLastData(data);
                    onData?.(data);
                } catch (err) {
                    const error = new Error('Failed to parse launch monitor data');
                    setError(error);
                    onError?.(error);
                }
            };

            ws.onerror = () => {
                const error = new Error('Launch monitor not available');
                setError(error);
                onError?.(error);
                setIsConnected(false);
            };

            ws.onclose = () => {
                setIsConnected(false);
            };

            return () => {
                ws.close();
            };
        } catch (err) {
            const error = err instanceof Error ? err : new Error('Failed to connect to launch monitor');
            setError(error);
            onError?.(error);
        }
    }, [enableRealDevice, onData, onError]);

    return {
        isConnected,
        lastData,
        error,
        simulateShot,
        connect // Available but requires enableRealDevice to be true
    };
};

export default useLaunchMonitor;
