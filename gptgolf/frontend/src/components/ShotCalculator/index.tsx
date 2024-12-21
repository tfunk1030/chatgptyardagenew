import React, { useState, useCallback } from 'react';
import styles from './ShotCalculator.module.css';
import DistanceInput from './DistanceInput';
import WeatherInfo from './WeatherInfo';
import AdjustedDistance from './AdjustedDistance';
import useWeather from '../../hooks/useWeather';

const ShotCalculator: React.FC = () => {
  const [targetDistance, setTargetDistance] = useState<number>(150);
  const [unit, setUnit] = useState<'yards' | 'meters'>('yards');
  const { weather, loading, error, refreshWeather, updateWeather } = useWeather();

  const handleDistanceChange = useCallback((distance: number) => {
    setTargetDistance(distance);
  }, []);

  const handleUnitChange = useCallback((newUnit: 'yards' | 'meters') => {
    // Convert the current distance when changing units
    const convertedDistance = newUnit === 'yards' 
      ? targetDistance * 1.09361  // meters to yards
      : targetDistance * 0.9144;  // yards to meters
    
    setTargetDistance(Math.round(convertedDistance));
    setUnit(newUnit);
  }, [targetDistance]);

  return (
    <div className={styles.container}>
      <h1 className={styles.title}>Golf Shot Calculator</h1>
      
      {error && (
        <div className={styles.error}>
          {error}
        </div>
      )}

      <div className={styles.content}>
        <div className={styles.inputSection}>
          <DistanceInput
            distance={targetDistance}
            unit={unit}
            onDistanceChange={handleDistanceChange}
            onUnitChange={handleUnitChange}
          />

          <WeatherInfo
            weather={weather}
            onRefresh={refreshWeather}
            onWeatherChange={updateWeather}
          />
        </div>

        {loading ? (
          <div className={styles.loading}>
            Loading weather data...
          </div>
        ) : (
          <AdjustedDistance
            actualDistance={targetDistance}
            unit={unit}
            weather={weather}
          />
        )}
      </div>

      <div className={styles.footer}>
        <p className={styles.lastUpdate}>
          Weather last updated: {weather.timestamp 
            ? new Date(weather.timestamp).toLocaleTimeString() 
            : 'Never'}
        </p>
        <button 
          className={styles.refreshButton}
          onClick={refreshWeather}
          disabled={loading}
        >
          ↻ Refresh Weather
        </button>
      </div>
    </div>
  );
};

export default ShotCalculator;
