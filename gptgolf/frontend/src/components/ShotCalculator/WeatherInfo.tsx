import React, { useState, useCallback } from 'react';
import styles from './WeatherInfo.module.css';

export interface WeatherData {
  temperature: number;    // Fahrenheit
  humidity: number;       // percentage
  pressure: number;       // inHg
  windSpeed: number;      // mph
  windDirection: number;  // degrees
  altitude: number;       // feet
}

interface ValidationRule {
  min: number;
  max: number;
  allowNegative: boolean;
  decimals: number;
  defaultValue: number;
  unit: string;
  label: string;
  tooltip: string;
}

type ValidationRules = Record<keyof WeatherData, ValidationRule>;

const validationRules: ValidationRules = {
  temperature: {
    min: -20,
    max: 120,
    allowNegative: true,
    decimals: 1,
    defaultValue: 72,
    unit: '°F',
    label: 'Temperature',
    tooltip: 'Air temperature affects ball flight distance'
  },
  humidity: {
    min: 0,
    max: 100,
    allowNegative: false,
    decimals: 1,
    defaultValue: 50,
    unit: '%',
    label: 'Humidity',
    tooltip: 'Higher humidity reduces air density and increases distance'
  },
  pressure: {
    min: 25,
    max: 35,
    allowNegative: false,
    decimals: 2,
    defaultValue: 29.92,
    unit: 'inHg',
    label: 'Pressure',
    tooltip: 'Barometric pressure impacts air density and ball flight'
  },
  windSpeed: {
    min: 0,
    max: 100,
    allowNegative: false,
    decimals: 1,
    defaultValue: 0,
    unit: 'mph',
    label: 'Wind Speed',
    tooltip: 'Wind speed significantly affects ball trajectory'
  },
  windDirection: {
    min: 0,
    max: 359,
    allowNegative: false,
    decimals: 0,
    defaultValue: 0,
    unit: '°',
    label: 'Wind Direction',
    tooltip: '0° is North, 90° is East, etc.'
  },
  altitude: {
    min: 0,
    max: 15000,
    allowNegative: false,
    decimals: 0,
    defaultValue: 0,
    unit: 'ft',
    label: 'Altitude',
    tooltip: 'Higher altitude means thinner air and longer shots'
  }
};

interface WeatherInfoProps {
  weather?: WeatherData;
  onRefresh?: () => void;
  onWeatherChange?: (updates: Partial<WeatherData>) => void;
}

const WeatherInfo: React.FC<WeatherInfoProps> = ({
  weather = {
    temperature: validationRules.temperature.defaultValue,
    humidity: validationRules.humidity.defaultValue,
    pressure: validationRules.pressure.defaultValue,
    windSpeed: validationRules.windSpeed.defaultValue,
    windDirection: validationRules.windDirection.defaultValue,
    altitude: validationRules.altitude.defaultValue
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

  const handleInputChange = useCallback((field: keyof WeatherData, value: string): void => {
    const rules = validationRules[field];

    // Handle special input states during typing
    const isTypingDecimal = value === '.' || value === '0.';
    const isTypingNegative = rules.allowNegative && value === '-';
    const isTypingZeroDecimal = value === '0.0' || (rules.allowNegative && value === '-0.0');
    const isTypingValidIntermediate = value.endsWith('.') && value.split('.')[0].length > 0;
    
    // Handle empty input or special typing states
    if (value === '') {
      onWeatherChange?.({ [field]: rules.defaultValue });
      return;
    } else if (rules.decimals > 0 && (isTypingDecimal || isTypingNegative || isTypingZeroDecimal || isTypingValidIntermediate)) {
      return; // Keep current value while user is typing
    }

    // Build pattern based on field rules
    const pattern = new RegExp(
      `^${rules.allowNegative ? '-?' : ''}\\d*${rules.decimals > 0 ? `\\.?\\d{0,${rules.decimals}}` : ''}$`
    );

    if (!pattern.test(value)) {
      return; // Invalid format
    }

    const numValue = parseFloat(value);
    if (!isNaN(numValue) && numValue >= rules.min && numValue <= rules.max) {
      // Round to specified decimal places
      const rounded = Number(numValue.toFixed(rules.decimals));
      onWeatherChange?.({ [field]: rounded });
    }
  }, [onWeatherChange]);


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
        {(Object.keys(validationRules) as Array<keyof WeatherData>).map((field) => {
          const rules = validationRules[field];
          return (
            <div key={field} className={styles['weather-item']}>
              <div className={styles['tooltip']}>{rules.tooltip}</div>
              <label htmlFor={field}>{rules.label}</label>
              <div className={styles['input-group']}>
                <input
                  id={field}
                  type="text"
                  inputMode="decimal"
                  pattern={`${rules.allowNegative ? '-?' : ''}\\d*${rules.decimals > 0 ? `\\.?\\d{0,${rules.decimals}}` : ''}`}
                  value={weather[field]}
                  onChange={(e) => handleInputChange(field, e.target.value)}
                  aria-label={field}
                  min={rules.min.toString()}
                  max={rules.max.toString()}
                  step={rules.decimals > 0 ? Math.pow(0.1, rules.decimals).toString() : '1'}
                />
                <span>{rules.unit}</span>
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
};

export default WeatherInfo;
