import React, { useState } from 'react';
import styles from './WeatherInfo.module.css';

interface WeatherData {
  temperature: number;    // Fahrenheit
  humidity: number;       // percentage
  pressure: number;       // inHg
  windSpeed: number;      // mph
  windDirection: number;  // degrees
  altitude: number;       // feet
}

interface WeatherInfoProps {
  weather?: WeatherData;
  onRefresh?: () => void;
  onWeatherChange?: (updates: Partial<WeatherData>) => void;
}

const WeatherInfo: React.FC<WeatherInfoProps> = ({ 
  weather = {
    temperature: 72,
    humidity: 50,
    pressure: 29.92,
    windSpeed: 0,
    windDirection: 0,
    altitude: 0
  },
  onRefresh,
  onWeatherChange
}) => {
  const [isRefreshing, setIsRefreshing] = useState(false);

  const handleRefresh = async () => {
    if (onRefresh) {
      setIsRefreshing(true);
      await onRefresh();
      setIsRefreshing(false);
    }
  };

  const handleInputChange = (field: keyof WeatherData, value: string) => {
    const numValue = parseFloat(value);
    if (!isNaN(numValue) && onWeatherChange) {
      onWeatherChange({ [field]: numValue });
    }
  };

  const getTooltip = (field: keyof WeatherData): string => {
    const tooltips = {
      temperature: 'Air temperature affects ball flight distance',
      humidity: 'Higher humidity reduces air density and increases distance',
      pressure: 'Barometric pressure impacts air density and ball flight',
      windSpeed: 'Wind speed significantly affects ball trajectory',
      windDirection: '0° is North, 90° is East, etc.',
      altitude: 'Higher altitude means thinner air and longer shots'
    };
    return tooltips[field];
  };

  return (
    <div className={styles['weather-info']} data-testid="weather-info">
      <div className={styles['weather-header']}>
        <h2 className={styles['weather-title']}>Weather Conditions</h2>
        <button 
          className={styles['refresh-button']}
          onClick={handleRefresh}
          disabled={isRefreshing}
        >
          {isRefreshing ? 'Updating...' : '↻ Refresh'}
        </button>
      </div>
      <div className={styles['weather-grid']}>
        <div className={styles['weather-item']}>
          <div className={styles['tooltip']}>{getTooltip('temperature')}</div>
          <label htmlFor="temperature">Temperature</label>
          <div className={styles['input-group']}>
            <input
              id="temperature"
              type="number"
              value={weather.temperature}
              onChange={(e) => handleInputChange('temperature', e.target.value)}
              aria-label="temperature"
              min="-20"
              max="120"
              step="1"
            />
            <span>°F</span>
          </div>
        </div>
        <div className={styles['weather-item']}>
          <div className={styles['tooltip']}>{getTooltip('windSpeed')}</div>
          <label htmlFor="windSpeed">Wind Speed</label>
          <div className={styles['input-group']}>
            <input
              id="windSpeed"
              type="number"
              value={weather.windSpeed}
              onChange={(e) => handleInputChange('windSpeed', e.target.value)}
              aria-label="wind speed"
              min="0"
              max="100"
              step="1"
            />
            <span>mph</span>
          </div>
        </div>
        <div className={styles['weather-item']}>
          <div className={styles['tooltip']}>{getTooltip('windDirection')}</div>
          <label htmlFor="windDirection">Wind Direction</label>
          <div className={styles['input-group']}>
            <input
              id="windDirection"
              type="number"
              value={weather.windDirection}
              onChange={(e) => handleInputChange('windDirection', e.target.value)}
              aria-label="wind direction"
              min="0"
              max="359"
              step="1"
            />
            <span>°</span>
          </div>
        </div>
        <div className={styles['weather-item']}>
          <div className={styles['tooltip']}>{getTooltip('humidity')}</div>
          <label htmlFor="humidity">Humidity</label>
          <div className={styles['input-group']}>
            <input
              id="humidity"
              type="number"
              value={weather.humidity}
              onChange={(e) => handleInputChange('humidity', e.target.value)}
              aria-label="humidity"
              min="0"
              max="100"
              step="1"
            />
            <span>%</span>
          </div>
        </div>
        <div className={styles['weather-item']}>
          <div className={styles['tooltip']}>{getTooltip('pressure')}</div>
          <label htmlFor="pressure">Pressure</label>
          <div className={styles['input-group']}>
            <input
              id="pressure"
              type="number"
              value={weather.pressure}
              onChange={(e) => handleInputChange('pressure', e.target.value)}
              aria-label="pressure"
              min="25"
              max="35"
              step="0.01"
            />
            <span>inHg</span>
          </div>
        </div>
        <div className={styles['weather-item']}>
          <div className={styles['tooltip']}>{getTooltip('altitude')}</div>
          <label htmlFor="altitude">Altitude</label>
          <div className={styles['input-group']}>
            <input
              id="altitude"
              type="number"
              value={weather.altitude}
              onChange={(e) => handleInputChange('altitude', e.target.value)}
              aria-label="altitude"
              min="0"
              max="15000"
              step="10"
            />
            <span>ft</span>
          </div>
        </div>
      </div>
    </div>
  );
};

export default WeatherInfo;
