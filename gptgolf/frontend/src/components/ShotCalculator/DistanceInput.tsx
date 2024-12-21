import React from 'react';
import styles from './DistanceInput.module.css';

interface DistanceInputProps {
  distance: number;
  unit: 'yards' | 'meters';
  onDistanceChange: (distance: number) => void;
  onUnitChange: (unit: 'yards' | 'meters') => void;
}

const commonDistances = [
  { label: '100', value: 100 },
  { label: '150', value: 150 },
  { label: '200', value: 200 },
];

const DistanceInput: React.FC<DistanceInputProps> = ({
  distance,
  unit,
  onDistanceChange,
  onUnitChange,
}) => {
  const handleInputChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = parseFloat(e.target.value);
    if (!isNaN(value)) {
      onDistanceChange(value);
    }
  };

  const handleUnitToggle = () => {
    const newUnit = unit === 'yards' ? 'meters' : 'yards';
    onUnitChange(newUnit);
  };

  const handleQuickSelect = (value: number) => {
    onDistanceChange(value);
  };

  return (
    <div className={styles.container}>
      <h2 className={styles.title}>Target Distance</h2>
      
      <div className={styles.inputGroup}>
        <input
          type="number"
          value={distance}
          onChange={handleInputChange}
          min="0"
          max="1000"
          step="1"
          className={styles.input}
          aria-label="Distance"
        />
        <button 
          onClick={handleUnitToggle}
          className={styles.unitToggle}
          aria-label="Toggle unit"
        >
          {unit}
        </button>
      </div>

      <div className={styles.quickSelect}>
        <p className={styles.quickSelectLabel}>Quick Select:</p>
        <div className={styles.quickSelectButtons}>
          {commonDistances.map(({ label, value }) => (
            <button
              key={value}
              onClick={() => handleQuickSelect(value)}
              className={styles.quickSelectButton}
            >
              {label} {unit}
            </button>
          ))}
        </div>
      </div>
    </div>
  );
};

export default DistanceInput;
