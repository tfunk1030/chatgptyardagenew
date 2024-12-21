import React, { useEffect, useRef, useState, useCallback } from 'react';
import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls';
import { Line2 } from 'three/examples/jsm/lines/Line2';
import { LineMaterial } from 'three/examples/jsm/lines/LineMaterial';
import { LineGeometry } from 'three/examples/jsm/lines/LineGeometry';
import styles from './ShotVisualizer.module.css';

interface Point3D {
    x: number;
    y: number;
    z: number;
}

interface TrajectoryPoint {
    position: Point3D;
    time: number;
    velocity: Point3D;
    spin: {
        rate: number;
        axis: number;
    };
}

interface ShotVisualizerProps {
    trajectory: TrajectoryPoint[];
    clubPath?: Point3D[];
    showGrid?: boolean;
    showAxes?: boolean;
    width?: number;
    height?: number;
    onFrameUpdate?: (frame: number) => void;
}

const ShotVisualizer: React.FC<ShotVisualizerProps> = ({
    trajectory,
    clubPath,
    showGrid = true,
    showAxes = true,
    width = 800,
    height = 600,
    onFrameUpdate
}) => {
    const containerRef = useRef<HTMLDivElement>(null);
    const rendererRef = useRef<THREE.WebGLRenderer | null>(null);
    const sceneRef = useRef<THREE.Scene | null>(null);
    const cameraRef = useRef<THREE.PerspectiveCamera | null>(null);
    const controlsRef = useRef<OrbitControls | null>(null);
    const ballRef = useRef<THREE.Mesh | null>(null);
    const trajectoryLineRef = useRef<Line2 | null>(null);
    const [isPlaying, setIsPlaying] = useState(false);
    const [currentFrame, setCurrentFrame] = useState(0);
    const [playbackSpeed, setPlaybackSpeed] = useState(1);
    const [showStats, setShowStats] = useState(true);

    const initializeScene = useCallback(() => {
        if (!containerRef.current) return;

        // Scene setup
        const scene = new THREE.Scene();
        scene.background = new THREE.Color(0xf0f0f0);
        sceneRef.current = scene;

        // Camera setup
        const camera = new THREE.PerspectiveCamera(75, width / height, 0.1, 1000);
        camera.position.set(5, 5, 5);
        cameraRef.current = camera;

        // Renderer setup
        const renderer = new THREE.WebGLRenderer({ antialias: true });
        renderer.setSize(width, height);
        renderer.setPixelRatio(window.devicePixelRatio);
        renderer.shadowMap.enabled = true;
        containerRef.current.appendChild(renderer.domElement);
        rendererRef.current = renderer;

        // Controls
        const controls = new OrbitControls(camera, renderer.domElement);
        controls.enableDamping = true;
        controls.dampingFactor = 0.05;
        controls.minDistance = 3;
        controls.maxDistance = 20;
        controlsRef.current = controls;

        // Ground plane
        const groundGeometry = new THREE.PlaneGeometry(100, 100);
        const groundMaterial = new THREE.MeshStandardMaterial({ 
            color: 0x90EE90,
            roughness: 0.8,
            metalness: 0.2
        });
        const ground = new THREE.Mesh(groundGeometry, groundMaterial);
        ground.rotation.x = -Math.PI / 2;
        ground.receiveShadow = true;
        scene.add(ground);

        // Grid
        if (showGrid) {
            const grid = new THREE.GridHelper(100, 100, 0x000000, 0x000000);
            grid.material.opacity = 0.2;
            grid.material.transparent = true;
            scene.add(grid);
        }

        // Axes
        if (showAxes) {
            const axes = new THREE.AxesHelper(5);
            scene.add(axes);
        }

        // Golf ball
        const ballGeometry = new THREE.SphereGeometry(0.0213, 32, 32);
        const ballMaterial = new THREE.MeshPhongMaterial({ 
            color: 0xffffff,
            specular: 0x444444,
            shininess: 60
        });
        const ball = new THREE.Mesh(ballGeometry, ballMaterial);
        ball.castShadow = true;
        scene.add(ball);
        ballRef.current = ball;

        // Lighting
        const ambientLight = new THREE.AmbientLight(0x404040, 0.5);
        scene.add(ambientLight);

        const directionalLight = new THREE.DirectionalLight(0xffffff, 0.8);
        directionalLight.position.set(5, 5, 5);
        directionalLight.castShadow = true;
        directionalLight.shadow.mapSize.width = 2048;
        directionalLight.shadow.mapSize.height = 2048;
        scene.add(directionalLight);

        // Trajectory line
        const positions: number[] = [];
        trajectory.forEach(point => {
            positions.push(point.position.x, point.position.y, point.position.z);
        });

        const lineGeometry = new LineGeometry();
        lineGeometry.setPositions(positions);

        const lineMaterial = new LineMaterial({
            color: 0x0088ff,
            linewidth: 3,
            resolution: new THREE.Vector2(width, height),
            dashed: false,
            alphaToCoverage: true
        });

        const trajectoryLine = new Line2(lineGeometry, lineMaterial);
        scene.add(trajectoryLine);
        trajectoryLineRef.current = trajectoryLine;

        // Club path visualization
        if (clubPath) {
            const clubPathGeometry = new LineGeometry();
            const clubPositions: number[] = [];
            clubPath.forEach(point => {
                clubPositions.push(point.x, point.y, point.z);
            });
            clubPathGeometry.setPositions(clubPositions);

            const clubPathMaterial = new LineMaterial({
                color: 0xff0000,
                linewidth: 2,
                resolution: new THREE.Vector2(width, height),
                dashed: true
            });

            const clubPathLine = new Line2(clubPathGeometry, clubPathMaterial);
            scene.add(clubPathLine);
        }

        return () => {
            renderer.dispose();
            if (containerRef.current) {
                containerRef.current.removeChild(renderer.domElement);
            }
        };
    }, [trajectory, clubPath, width, height, showGrid, showAxes]);

    useEffect(() => {
        const cleanup = initializeScene();
        return cleanup;
    }, [initializeScene]);

    useEffect(() => {
        const animate = () => {
            requestAnimationFrame(animate);

            if (isPlaying && currentFrame < trajectory.length) {
                const frameIncrement = playbackSpeed;
                const nextFrame = Math.min(currentFrame + frameIncrement, trajectory.length - 1);
                
                const point = trajectory[Math.floor(nextFrame)];
                if (ballRef.current && point) {
                    ballRef.current.position.set(
                        point.position.x,
                        point.position.y,
                        point.position.z
                    );

                    const spinAxis = new THREE.Vector3(
                        Math.cos(point.spin.axis),
                        0,
                        Math.sin(point.spin.axis)
                    );
                    ballRef.current.rotateOnAxis(spinAxis, point.spin.rate * 0.1);
                }

                onFrameUpdate?.(Math.floor(nextFrame));
                setCurrentFrame(nextFrame);
            }

            if (controlsRef.current) {
                controlsRef.current.update();
            }

            if (rendererRef.current && sceneRef.current && cameraRef.current) {
                rendererRef.current.render(sceneRef.current, cameraRef.current);
            }
        };

        animate();
    }, [isPlaying, currentFrame, trajectory, playbackSpeed, onFrameUpdate]);

    const handlePlayPause = () => {
        setIsPlaying(!isPlaying);
    };

    const handleReset = () => {
        setCurrentFrame(0);
        setIsPlaying(false);
        if (ballRef.current) {
            ballRef.current.position.set(
                trajectory[0].position.x,
                trajectory[0].position.y,
                trajectory[0].position.z
            );
        }
    };

    const handleSpeedChange = (speed: number) => {
        setPlaybackSpeed(speed);
    };

    const getTrajectoryStats = () => {
        if (currentFrame >= trajectory.length) return null;
        const point = trajectory[Math.floor(currentFrame)];
        return {
            height: point.position.y.toFixed(1),
            distance: Math.sqrt(point.position.x ** 2 + point.position.z ** 2).toFixed(1),
            speed: Math.sqrt(
                point.velocity.x ** 2 + 
                point.velocity.y ** 2 + 
                point.velocity.z ** 2
            ).toFixed(1),
            spinRate: point.spin.rate.toFixed(0)
        };
    };

    const stats = getTrajectoryStats();

    return (
        <div className={styles.container}>
            <div className={styles.canvas} ref={containerRef}>
                {showStats && stats && (
                    <div className={styles['stats-overlay']}>
                        <div className={styles['stats-item']}>
                            <span className={styles['stats-label']}>Height:</span>
                            <span className={styles['stats-value']}>{stats.height} yards</span>
                        </div>
                        <div className={styles['stats-item']}>
                            <span className={styles['stats-label']}>Distance:</span>
                            <span className={styles['stats-value']}>{stats.distance} yards</span>
                        </div>
                        <div className={styles['stats-item']}>
                            <span className={styles['stats-label']}>Ball Speed:</span>
                            <span className={styles['stats-value']}>{stats.speed} mph</span>
                        </div>
                        <div className={styles['stats-item']}>
                            <span className={styles['stats-label']}>Spin Rate:</span>
                            <span className={styles['stats-value']}>{stats.spinRate} rpm</span>
                        </div>
                    </div>
                )}
            </div>
            <div className={styles.controls}>
                <button 
                    className={styles['control-button']}
                    onClick={handlePlayPause}
                    aria-label={isPlaying ? 'Pause' : 'Play'}
                >
                    {isPlaying ? '⏸️ Pause' : '▶️ Play'}
                </button>
                <button 
                    className={styles['control-button']}
                    onClick={handleReset}
                    aria-label="Reset"
                >
                    🔄 Reset
                </button>
                <div className={styles.progress}>
                    <span>Frame: {Math.floor(currentFrame)} / {trajectory.length}</span>
                    <div className={styles['progress-bar']}>
                        <div 
                            className={styles['progress-fill']}
                            style={{ width: `${(currentFrame / trajectory.length) * 100}%` }}
                        />
                    </div>
                </div>
                <select
                    value={playbackSpeed}
                    onChange={(e) => handleSpeedChange(Number(e.target.value))}
                    className={styles['control-button']}
                    aria-label="Playback speed"
                >
                    <option value={0.5}>0.5x</option>
                    <option value={1}>1x</option>
                    <option value={2}>2x</option>
                    <option value={4}>4x</option>
                </select>
                <button
                    className={styles['control-button']}
                    onClick={() => setShowStats(!showStats)}
                    aria-label={showStats ? 'Hide Stats' : 'Show Stats'}
                >
                    {showStats ? '📊 Hide Stats' : '📊 Show Stats'}
                </button>
            </div>
        </div>
    );
};

export default ShotVisualizer;
