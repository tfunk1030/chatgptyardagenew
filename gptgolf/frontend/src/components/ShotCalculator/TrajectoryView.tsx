import React from 'react';
import styles from './ShotCalculator.module.css';

interface TrajectoryPoint {
  distance: number;
  height: number;
  lateralDistance: number;
}

interface TrajectoryViewProps {
  trajectory?: TrajectoryPoint[];
}

const TrajectoryView: React.FC<TrajectoryViewProps> = ({ trajectory = [] }) => {
  // Default path for visualization
  const defaultPath = "M 50,250 Q 250,50 450,250";
  
  return (
    <div className={styles['trajectory-view']}>
      <h2>Shot Trajectory</h2>
      <div className={styles['trajectory-canvas']}>
        <svg 
          width="100%" 
          height="100%" 
          viewBox="0 0 500 300"
          data-testid="trajectory-view"
        >
          <path
            d={defaultPath}
            stroke="#2196f3"
            strokeWidth="2"
            fill="none"
          />
        </svg>
      </div>
      <div className={styles['trajectory-stats']}>
        <div className={styles.stat}>
          <label>Carry Distance:</label>
          <span>230 yards</span>
        </div>
        <div className={styles.stat}>
          <label>Total Distance:</label>
          <span>245 yards</span>
        </div>
        <div className={styles.stat}>
          <label>Apex:</label>
          <span>95 feet</span>
        </div>
      </div>
    </div>
  );
};

export default TrajectoryView;
