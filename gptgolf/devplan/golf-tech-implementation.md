# Golf Shot Calculator - Technical Implementation Guide

## Core Requirements

### Technical Stack
- Frontend: TypeScript + React
- State Management: React Context + Hooks
- API Integration: Axios
- Data Storage: IndexedDB + LocalStorage
- Worker Threads for Physics Calculations
- PWA Capability Required

### Key Components

#### 1. Physics Engine (Priority)
```typescript
interface BallState {
    velocity: number;      // ft/s
    spinRate: number;      // rpm
    height: number;        // feet
    distance: number;      // yards
    lateralDistance: number; // yards
    angle: number;         // degrees
}

interface ShotConditions {
    temperature: number;   // Fahrenheit
    humidity: number;      // percentage
    pressure: number;      // inHg
    altitude: number;      // feet
    windSpeed: number;     // mph
    windDirection: number; // degrees
}
```

Core Calculations Required:
1. Magnus Effect + Lift Coefficient
2. Drag Coefficient with Reynolds Number
3. Wind Gradient with Altitude
4. Air Density Effects
5. Spin Decay Over Distance

#### 2. Data Management
```typescript
interface ShotData {
    clubData: typeof PGA_CLUB_DATA[keyof typeof PGA_CLUB_DATA];
    conditions: ShotConditions;
    result: {
        carry: number;
        total: number;
        apex: number;
        landingAngle: number;
        trajectory: BallState[];
    };
}
```

Storage Requirements:
- Cache weather data (30-minute expiry)
- Store user club data
- Save shot history
- Maintain correction factors

#### 3. Weather Integration
- Primary: Tomorrow.io API
- Fallback: Local cache + user input
- Required fields: temp, humidity, pressure, wind
- Update frequency: 30 minutes
- Error handling for API failures

## Implementation Phases

### Phase 1: Core Physics (Week 1-2)
1. Implement ball flight calculations:
```javascript
function calculateTrajectory(initialConditions: BallState, conditions: ShotConditions): BallState[] {
    const dt = 0.001; // 1ms time steps
    const trajectory: BallState[] = [];
    let currentState = {...initialConditions};

    while (currentState.height >= 0) {
        const forces = calculateForces(currentState, conditions);
        currentState = updateState(currentState, forces, dt);
        trajectory.push({...currentState});
    }

    return trajectory;
}
```

2. Implement environmental adjustments
3. Add club-specific calculations
4. Create validation system

### Phase 2: Data Integration (Week 2-3)
1. Set up weather API integration
2. Implement data storage
3. Create club management system
4. Add shot history tracking

### Phase 3: UI Development (Week 3-4)
1. Build trajectory visualization
2. Create input forms
3. Add real-time updates
4. Implement club selector

## Key Files to Create

### Core Calculation Files
1. `src/physics/trajectory.ts`
   - Ball flight calculations
   - Force computations
   - Environmental adjustments

2. `src/physics/environment.ts`
   - Air density calculations
   - Wind effects
   - Temperature impacts

3. `src/data/shot-tracker.ts`
   - Shot history
   - Statistics
   - Data validation

### Component Structure
```
src/
├── components/
│   ├── ShotCalculator/
│   │   ├── index.tsx
│   │   ├── TrajectoryView.tsx
│   │   ├── WeatherInfo.tsx
│   │   └── ClubSelector.tsx
│   ├── Weather/
│   │   ├── index.tsx
│   │   └── WeatherDisplay.tsx
│   └── Clubs/
│       ├── index.tsx
│       └── ClubManager.tsx
├── physics/
│   ├── trajectory.ts
│   ├── environment.ts
│   └── constants.ts
├── data/
│   ├── shot-tracker.ts
│   ├── weather-api.ts
│   └── storage.ts
└── utils/
    ├── calculations.ts
    └── validation.ts
```

## Critical Functions

### Ball Flight Calculation
```typescript
function calculateBallFlight(
    club: ClubData,
    conditions: ShotConditions,
    corrections: CorrectionFactors
): ShotResult {
    // Initialize with PGA baseline data
    const baseConditions = PGA_CLUB_DATA[