import React from 'react';
import { ErrorBoundary } from './components/ErrorBoundary';
import ShotCalculator from './components/ShotCalculator';
import './App.css';

const App: React.FC = () => {
  return (
    <div className="app">
      <ErrorBoundary>
        <ShotCalculator />
      </ErrorBoundary>
    </div>
  );
};

export default App;
