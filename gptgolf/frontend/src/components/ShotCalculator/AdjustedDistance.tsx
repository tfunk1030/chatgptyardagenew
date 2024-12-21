import React from 'react';
import styles from './AdjustedDistance.module.css';

interface WeatherEffect {
  factor: number;
  description: string;
}

interface AdjustedDistanceProps {
  actualDistance: number;
  unit: 'yards' | 'meters';
  weather: {
    temperature: number;
    humidity: number;
    pressure: number;
    windSpeed: number;
    windDirection: number;
    altitude: number;
  };
  shotType?: 'low' | 'normal' | 'high';
  terrain?: 'water' | 'fairway' | 'rough';
}

const AdjustedDistance: React.FC<AdjustedDistanceProps> = ({
  actualDistance,
  unit,
  weather,
  shotType = 'normal',
  terrain = 'fairway'
}) => {
  // Calculate weather effects
  const calculateWeatherEffects = (): WeatherEffect[] => {
    const effects: WeatherEffect[] = [];
    
    // Temperature effect (based on standard temp of 72°F)
    // Higher temperature = less dense air = ball goes shorter
    const tempDiff = weather.temperature - 72;
    const tempEffect = -tempDiff * 0.15; // -0.15% per degree F above standard
    effects.push({
      factor: tempEffect,
      description: `${weather.temperature}°F: ${Math.abs(tempEffect).toFixed(1)}% ${tempDiff > 0 ? 'shorter' : 'longer'} (${tempDiff > 0 ? 'less' : 'more'} dense air)`
    });

    // Air pressure effect (based on standard pressure of 29.92 inHg)
    // Higher pressure = denser air = ball goes shorter
    const pressureDiff = weather.pressure - 29.92;
    const pressureEffect = -pressureDiff * 2.5; // -2.5% per inHg above standard
    effects.push({
      factor: pressureEffect,
      description: `${weather.pressure} inHg: ${Math.abs(pressureEffect).toFixed(1)}% ${pressureDiff > 0 ? 'shorter' : 'longer'} (${pressureDiff > 0 ? 'more' : 'less'} dense air)`
    });

    // Altitude effect (based on sea level)
    // Higher altitude = less dense air = ball goes shorter
    const altEffect = -weather.altitude * 0.002; // -0.2% per 100 feet
    effects.push({
      factor: altEffect,
      description: `${weather.altitude}ft elevation: ${Math.abs(altEffect).toFixed(1)}% shorter (thinner air)`
    });

    // Wind effect
    const windEffect = calculateWindEffect(weather.windSpeed, weather.windDirection);
    effects.push({
      factor: windEffect.factor,
      description: windEffect.description
    });

    // Humidity effect (based on 50% standard)
    // Higher humidity = less dense air = ball goes shorter
    const humidityDiff = weather.humidity - 50;
    const humidityEffect = -humidityDiff * 0.05; // -0.05% per % above standard
    effects.push({
      factor: humidityEffect,
      description: `${weather.humidity}% humidity: ${Math.abs(humidityEffect).toFixed(1)}% ${humidityDiff > 0 ? 'shorter' : 'longer'} (${humidityDiff > 0 ? 'less' : 'more'} dense air)`
    });

    return effects;
  };

  const calculateWindEffect = (speed: number, direction: number): WeatherEffect => {
    // Convert wind direction to radians and calculate components
    const windRad = (direction * Math.PI) / 180;
    const headwind = -speed * Math.cos(windRad);
    const crosswind = speed * Math.sin(windRad);
    
    // Shot type multipliers
    const shotTypeMultipliers = {
      low: 0.7,    // Low shots less affected
      normal: 1.0,
      high: 1.4    // High shots more affected
    } as const;
    
    // Terrain roughness factors
    const terrainFactors = {
      water: 1.2,    // More wind effect over water
      fairway: 1.0,
      rough: 0.8     // Less wind effect in rough
    } as const;
    
    // Height-based scaling (assuming average max height based on shot type)
    const maxHeightMultipliers = {
      low: 0.8,
      normal: 1.0,
      high: 1.3
    } as const;
    
    const shotTypeMultiplier = shotTypeMultipliers[shotType];
    const terrainFactor = terrainFactors[terrain];
    const maxHeightMultiplier = maxHeightMultipliers[shotType];
    
    // Enhanced coefficients based on physics engine
    const headwindCoeff = 0.35 * shotTypeMultiplier * terrainFactor;  // Increased from 0.3
    const crosswindCoeff = 0.15 * maxHeightMultiplier * terrainFactor; // Increased from 0.1
    
    // Calculate effects with enhanced precision
    const headwindEffect = headwind * headwindCoeff;
    const crosswindEffect = -Math.abs(crosswind) * crosswindCoeff;
    
    // Apply non-linear scaling for strong winds (> 15mph)
    const windStrengthMultiplier = speed > 15 ? 1 + (speed - 15) * 0.02 : 1;
    const totalEffect = (headwindEffect + crosswindEffect) * windStrengthMultiplier;
    
    let description = '';
    if (Math.abs(headwind) > Math.abs(crosswind)) {
      if (headwind > 0) {
        description = `${speed}mph headwind: plays ${Math.abs(totalEffect).toFixed(1)}% longer`;
      } else {
        description = `${speed}mph tailwind: plays ${Math.abs(totalEffect).toFixed(1)}% shorter`;
      }
    } else {
      description = `${speed}mph crosswind: plays ${Math.abs(totalEffect).toFixed(1)}% shorter (increased drag)`;
    }

    return { factor: totalEffect, description };
  };

  // Calculate total adjustment
  const effects = calculateWeatherEffects();
  const totalAdjustment = effects.reduce((sum, effect) => sum + effect.factor, 0);
  const adjustedDistance = actualDistance * (1 + totalAdjustment / 100);

  return (
    <div className={styles.container}>
      <div className={styles.distances}>
        <div className={styles.actual}>
          <h3>Actual Distance</h3>
          <span className={styles.value}>{actualDistance} {unit}</span>
        </div>
        <div className={styles.arrow}>→</div>
        <div className={styles.adjusted}>
          <h3>Plays Like</h3>
          <span className={styles.value}>{Math.round(adjustedDistance)} {unit}</span>
          <span className={styles.difference}>
            {totalAdjustment > 0 ? '+' : ''}{Math.round(adjustedDistance - actualDistance)} {unit}
          </span>
        </div>
      </div>

      <div className={styles.effects}>
        <h3>Environmental Effects</h3>
        <ul>
          {effects.map((effect, index) => (
            <li key={index} className={styles.effect}>
              {effect.description}
            </li>
          ))}
        </ul>
      </div>

      <div className={styles.summary}>
        <h3>Total Adjustment</h3>
        <span className={styles.total}>
          Plays {Math.abs(totalAdjustment).toFixed(1)}% {totalAdjustment > 0 ? 'longer' : 'shorter'}
        </span>
      </div>
    </div>
  );
};

export default AdjustedDistance;
