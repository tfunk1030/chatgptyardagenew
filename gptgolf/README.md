# Golf Shot Calculator

A modern web application that helps golfers calculate adjusted shot distances based on current weather conditions. The app provides real-time weather data and explains how environmental factors affect shot distance.

## Features

- Input target distance with quick presets
- Automatic weather data fetching based on location
- Detailed breakdown of environmental effects:
  - Temperature impact
  - Wind (headwind/tailwind/crosswind)
  - Altitude effect
  - Air pressure influence
- Clear visualization of adjusted playing distance
- Explanation of all factors affecting the shot

## Setup

### Prerequisites

- Node.js (v14 or higher)
- npm or yarn
- Tomorrow.io API key for weather data

### Backend Setup

1. Navigate to the server directory:
```bash
cd server
```

2. Install dependencies:
```bash
npm install
```

3. Create environment file:
```bash
cp .env.example .env
```

4. Edit `.env` and add your Tomorrow.io API key:
```
TOMORROW_IO_API_KEY=your_api_key_here
```

5. Start the server:
```bash
npm run dev
```

The server will run on port 3001 by default.

### Frontend Setup

1. Navigate to the frontend directory:
```bash
cd frontend
```

2. Install dependencies:
```bash
npm install
```

3. Start the development server:
```bash
npm start
```

The frontend will run on port 3000 by default.

## Usage

1. Allow location access when prompted (for automatic weather data)
2. Enter your target shot distance
3. View the adjusted playing distance and detailed breakdown of environmental effects
4. Weather data automatically refreshes every 15 minutes

## Development

### Project Structure

```
gptgolf/
├── frontend/           # React frontend application
│   ├── src/
│   │   ├── components/  # React components
│   │   ├── hooks/      # Custom React hooks
│   │   └── utils/      # Utility functions
│   └── public/         # Static assets
├── server/            # Express backend server
│   ├── server.js      # Main server file
│   └── package.json   # Server dependencies
└── README.md         # This file
```

### Key Components

- `ShotCalculator`: Main component for distance calculation
- `DistanceInput`: Handles distance input with presets
- `WeatherInfo`: Displays current weather conditions
- `AdjustedDistance`: Shows calculated results and explanations

### Weather Data

The application uses Tomorrow.io's API to fetch real-time weather data, including:
- Temperature
- Wind speed and direction
- Air pressure
- Humidity

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.
