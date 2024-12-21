import { useState, useEffect, useCallback } from 'react';

interface WeatherData {
  temperature: number;    // Fahrenheit
  humidity: number;       // percentage
  pressure: number;      // inHg
  windSpeed: number;     // mph
  windDirection: number; // degrees
  altitude: number;      // feet
  timestamp?: number;    // Unix timestamp
}

interface UseWeatherResult {
  weather: WeatherData;
  loading: boolean;
  error: string | null;
  refreshWeather: () => Promise<void>;
  updateWeather: (updates: Partial<WeatherData>) => void;
}

const defaultWeather: WeatherData = {
  temperature: 72,
  humidity: 50,
  pressure: 29.92,
  windSpeed: 0,
  windDirection: 0,
  altitude: 0
};

export function useWeather(): UseWeatherResult {
  const [weather, setWeather] = useState<WeatherData>(defaultWeather);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const fetchWeather = useCallback(async (latitude: number, longitude: number) => {
    try {
      const response = await fetch(`/api/weather?lat=${latitude}&lon=${longitude}`);
      if (!response.ok) {
        throw new Error('Failed to fetch weather data');
      }
      const data = await response.json();
      setWeather({
        temperature: data.temperature,
        humidity: data.humidity,
        pressure: data.pressure,
        windSpeed: data.windSpeed,
        windDirection: data.windDirection,
        altitude: data.altitude,
        timestamp: Date.now()
      });
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to fetch weather');
      // Fall back to stored weather data if available
      const storedWeather = localStorage.getItem('lastWeather');
      if (storedWeather) {
        setWeather(JSON.parse(storedWeather));
      }
    }
  }, []);

  const refreshWeather = useCallback(async () => {
    setLoading(true);
    setError(null);

    try {
      // Get user's location
      const position = await new Promise<GeolocationPosition>((resolve, reject) => {
        navigator.geolocation.getCurrentPosition(resolve, reject);
      });

      await fetchWeather(position.coords.latitude, position.coords.longitude);
    } catch (err) {
      setError('Unable to get location or weather data');
    } finally {
      setLoading(false);
    }
  }, [fetchWeather]);

  const updateWeather = useCallback((updates: Partial<WeatherData>) => {
    setWeather(prev => {
      const updated = { ...prev, ...updates };
      localStorage.setItem('lastWeather', JSON.stringify(updated));
      return updated;
    });
  }, []);

  // Initial weather fetch
  useEffect(() => {
    const storedWeather = localStorage.getItem('lastWeather');
    if (storedWeather) {
      setWeather(JSON.parse(storedWeather));
    }
    refreshWeather();
  }, [refreshWeather]);

  // Auto-refresh weather every 15 minutes
  useEffect(() => {
    const interval = setInterval(refreshWeather, 15 * 60 * 1000);
    return () => clearInterval(interval);
  }, [refreshWeather]);

  return {
    weather,
    loading,
    error,
    refreshWeather,
    updateWeather
  };
}

export default useWeather;
