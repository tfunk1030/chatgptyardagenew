import React from 'react';
import styles from './ShotAnalysis.module.css';


interface ShotData {
  distance: number;
  height: number;
  spinRate: number;
  launchAngle: number;
  ballSpeed: number;
  flightTime: number;
}

interface WindData {
  speed: number;
  direction: number;
  altitude: number;
  temperature: number;
}

interface ShotAnalysisProps {
  shotData?: ShotData;
  windData?: WindData;
  previousShots?: ShotData[];
}

const ShotAnalysis: React.FC<ShotAnalysisProps> = ({
  shotData,
  windData,
  previousShots = []
}) => {
  if (!shotData) {
    return null;
  }

  // Calculate efficiency metrics
  const calculateEfficiency = () => {
    const optimalLaunchAngle = 12.0; // Typical optimal launch angle
    const optimalSpinRate = 2500; // Typical optimal spin rate
    
    const launchAngleEff = 100 - Math.abs(shotData.launchAngle - optimalLaunchAngle) * 5;
    const spinRateEff = 100 - Math.abs(shotData.spinRate - optimalSpinRate) / 50;
    
    return {
      launchAngleEfficiency: Math.max(0, Math.min(100, launchAngleEff)),
      spinRateEfficiency: Math.max(0, Math.min(100, spinRateEff))
    };
  };

  // Calculate environmental impact
  const calculateWindEffect = () => {
    if (!windData) return null;

    const headwindComponent = Math.cos(windData.direction * Math.PI / 180) * windData.speed;
    const crosswindComponent = Math.sin(windData.direction * Math.PI / 180) * windData.speed;

    return {
      carryEffect: -headwindComponent * 2.0, // Approximate yards lost/gained
      dispersionEffect: crosswindComponent * 1.5 // Approximate yards of lateral movement
    };
  };

  // Compare with previous shots
  const getHistoricalComparison = () => {
    if (previousShots.length === 0) return null;

    const avgDistance = previousShots.reduce((sum, shot) => sum + shot.distance, 0) / previousShots.length;
    const avgHeight = previousShots.reduce((sum, shot) => sum + shot.height, 0) / previousShots.length;
    
    return {
      distanceDiff: shotData.distance - avgDistance,
      heightDiff: shotData.height - avgHeight
    };
  };

  const efficiency = calculateEfficiency();
  const windEffect = calculateWindEffect();
  const historical = getHistoricalComparison();

  return (
    <div className={styles['shot-analysis']}>
      <h3 className={styles.title}>Shot Analysis</h3>
      
      <div className={styles['analysis-section']}>
        <h4 className={styles['section-title']}>Shot Metrics</h4>
        <div className={styles.metric}>
          <label className={styles.label}>Ball Speed:</label>
          <span className={styles.value}>{shotData.ballSpeed.toFixed(1)} mph</span>
        </div>
        <div className={styles.metric}>
          <label className={styles.label}>Launch Angle:</label>
          <span className={styles.value}>{shotData.launchAngle.toFixed(1)}°</span>
        </div>
        <div className={styles.metric}>
          <label className={styles.label}>Spin Rate:</label>
          <span className={styles.value}>{shotData.spinRate.toFixed(0)} rpm</span>
        </div>
      </div>

      <div className={styles['analysis-section']}>
        <h4 className={styles['section-title']}>Efficiency Ratings</h4>
        <div className={styles.metric}>
          <label className={styles.label}>Launch Efficiency:</label>
          <span className={styles.value}>{efficiency.launchAngleEfficiency.toFixed(1)}%</span>
        </div>
        <div className={styles.metric}>
          <label className={styles.label}>Spin Efficiency:</label>
          <span className={styles.value}>{efficiency.spinRateEfficiency.toFixed(1)}%</span>
        </div>
      </div>

      {windEffect && (
        <div className={styles['analysis-section']}>
          <h4 className={styles['section-title']}>Wind Impact</h4>
          <div className={styles.metric}>
            <label className={styles.label}>Carry Effect:</label>
            <span className={styles.value}>{windEffect.carryEffect > 0 ? '+' : ''}{windEffect.carryEffect.toFixed(1)} yards</span>
          </div>
          <div className={styles.metric}>
            <label className={styles.label}>Side Movement:</label>
            <span className={styles.value}>{windEffect.dispersionEffect > 0 ? 'Right ' : 'Left '}{Math.abs(windEffect.dispersionEffect).toFixed(1)} yards</span>
          </div>
        </div>
      )}

      {historical && (
        <div className={styles['analysis-section']}>
          <h4 className={styles['section-title']}>Historical Comparison</h4>
          <div className={styles.metric}>
            <label className={styles.label}>Distance vs Avg:</label>
            <span className={styles.value}>{historical.distanceDiff > 0 ? '+' : ''}{historical.distanceDiff.toFixed(1)} yards</span>
          </div>
          <div className={styles.metric}>
            <label className={styles.label}>Height vs Avg:</label>
            <span className={styles.value}>{historical.heightDiff > 0 ? '+' : ''}{historical.heightDiff.toFixed(1)} feet</span>
          </div>
        </div>
      )}
    </div>
  );
};

export default ShotAnalysis;
