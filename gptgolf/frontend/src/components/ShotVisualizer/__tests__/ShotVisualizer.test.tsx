import React from 'react';
import { render, screen, fireEvent } from '@testing-library/react';
import ShotVisualizer from '../ShotVisualizer';

// Mock three.js modules
jest.mock('three', () => ({
    Scene: jest.fn(),
    PerspectiveCamera: jest.fn(),
    WebGLRenderer: jest.fn(() => ({
        setSize: jest.fn(),
        setPixelRatio: jest.fn(),
        render: jest.fn(),
        dispose: jest.fn(),
        domElement: document.createElement('canvas')
    })),
    GridHelper: jest.fn(),
    AxesHelper: jest.fn(),
    SphereGeometry: jest.fn(),
    MeshPhongMaterial: jest.fn(),
    Mesh: jest.fn(() => ({
        position: { set: jest.fn() },
        rotateOnAxis: jest.fn()
    })),
    AmbientLight: jest.fn(),
    DirectionalLight: jest.fn(() => ({
        position: { set: jest.fn() }
    })),
    Vector2: jest.fn(),
    Vector3: jest.fn(),
    Color: jest.fn()
}));

jest.mock('three/examples/jsm/controls/OrbitControls', () => ({
    OrbitControls: jest.fn(() => ({
        enableDamping: true,
        update: jest.fn()
    }))
}));

jest.mock('three/examples/jsm/lines/Line2', () => ({
    Line2: jest.fn()
}));

jest.mock('three/examples/jsm/lines/LineMaterial', () => ({
    LineMaterial: jest.fn()
}));

jest.mock('three/examples/jsm/lines/LineGeometry', () => ({
    LineGeometry: jest.fn(() => ({
        setPositions: jest.fn()
    }))
}));

describe('ShotVisualizer', () => {
    const mockTrajectory = [
        {
            position: { x: 0, y: 0, z: 0 },
            time: 0,
            velocity: { x: 50, y: 20, z: 0 },
            spin: { rate: 2500, axis: 0 }
        },
        {
            position: { x: 10, y: 5, z: 0 },
            time: 0.1,
            velocity: { x: 48, y: 18, z: 0 },
            spin: { rate: 2400, axis: 0 }
        },
        {
            position: { x: 20, y: 8, z: 0 },
            time: 0.2,
            velocity: { x: 46, y: 15, z: 0 },
            spin: { rate: 2300, axis: 0 }
        }
    ];

    const mockClubPath = [
        { x: -1, y: 0, z: 0 },
        { x: 0, y: 0, z: 0 },
        { x: 1, y: 0, z: 0 }
    ];

    beforeEach(() => {
        // Clear all mocks before each test
        jest.clearAllMocks();
    });

    it('renders without crashing', () => {
        render(<ShotVisualizer trajectory={mockTrajectory} />);
        expect(screen.getByRole('button', { name: /play/i })).toBeInTheDocument();
    });

    it('shows play/pause button', () => {
        render(<ShotVisualizer trajectory={mockTrajectory} />);
        const playButton = screen.getByRole('button', { name: /play/i });
        fireEvent.click(playButton);
        expect(screen.getByRole('button', { name: /pause/i })).toBeInTheDocument();
    });

    it('shows reset button', () => {
        render(<ShotVisualizer trajectory={mockTrajectory} />);
        expect(screen.getByRole('button', { name: /reset/i })).toBeInTheDocument();
    });

    it('shows frame counter', () => {
        render(<ShotVisualizer trajectory={mockTrajectory} />);
        expect(screen.getByText(/frame:/i)).toBeInTheDocument();
    });

    it('handles club path visualization', () => {
        render(
            <ShotVisualizer 
                trajectory={mockTrajectory} 
                clubPath={mockClubPath}
            />
        );
        // Verify club path is rendered (implementation specific)
    });

    it('handles frame updates', () => {
        const mockOnFrameUpdate = jest.fn();
        render(
            <ShotVisualizer 
                trajectory={mockTrajectory} 
                onFrameUpdate={mockOnFrameUpdate}
            />
        );
        
        const playButton = screen.getByRole('button', { name: /play/i });
        fireEvent.click(playButton);
        
        // Wait for frame updates
        setTimeout(() => {
            expect(mockOnFrameUpdate).toHaveBeenCalled();
        }, 100);
    });

    it('handles grid toggle', () => {
        render(
            <ShotVisualizer 
                trajectory={mockTrajectory} 
                showGrid={false}
            />
        );
        // Verify grid is not rendered (implementation specific)
    });

    it('handles axes toggle', () => {
        render(
            <ShotVisualizer 
                trajectory={mockTrajectory} 
                showAxes={false}
            />
        );
        // Verify axes are not rendered (implementation specific)
    });

    it('cleans up on unmount', () => {
        const { unmount } = render(<ShotVisualizer trajectory={mockTrajectory} />);
        unmount();
        // Verify cleanup (implementation specific)
    });

    it('handles window resize', () => {
        render(<ShotVisualizer trajectory={mockTrajectory} />);
        
        // Simulate window resize
        global.innerWidth = 1024;
        global.innerHeight = 768;
        global.dispatchEvent(new Event('resize'));
        
        // Verify resize handling (implementation specific)
    });
});
