const express = require('express');
const cors = require('cors');
const axios = require('axios');
require('dotenv').config();

const app = express();
const port = process.env.PORT || 4000;

app.use(cors());
app.use(express.json());

// Mock weather data for development
const mockWeather = {
  temperature: 72,
  humidity: 50,
  pressure: 29.92,
  windSpeed: 10,
  windDirection: 45,
  altitude: 100,
  timestamp: Date.now()
};

// Weather API endpoint
app.get('/api/weather', async (req, res) => {
  try {
    const { lat, lon } = req.query;
    
    if (!lat || !lon) {
      return res.status(400).json({ error: 'Latitude and longitude are required' });
    }

    // Use mock data for development
    if (!process.env.TOMORROW_IO_API_KEY || process.env.NODE_ENV === 'development') {
      return res.json({
        ...mockWeather,
        timestamp: Date.now()
      });
    }

    const response = await axios.get(
      `https://api.tomorrow.io/v4/weather/realtime`,
      {
        params: {
          location: `${lat},${lon}`,
          apikey: process.env.TOMORROW_IO_API_KEY,
          units: 'imperial'
        }
      }
    );

    // Transform the response to match our frontend expectations
    const data = response.data.data.values;
    res.json({
      temperature: data.temperature,
      humidity: data.humidity,
      pressure: data.pressureSeaLevel,
      windSpeed: data.windSpeed,
      windDirection: data.windDirection,
      altitude: 0, // This would need to come from a terrain API
      timestamp: Date.now()
    });
  } catch (error) {
    console.error('Weather API Error:', error.response?.data || error.message);
    // Return mock data on error in development
    if (process.env.NODE_ENV === 'development') {
      return res.json({
        ...mockWeather,
        timestamp: Date.now()
      });
    }
    res.status(500).json({ 
      error: 'Failed to fetch weather data',
      details: process.env.NODE_ENV === 'development' ? error.message : undefined
    });
  }
});

// Health check endpoint
app.get('/health', (req, res) => {
  res.json({ status: 'ok' });
});

app.listen(port, () => {
  console.log(`Server running on port ${port}`);
});
