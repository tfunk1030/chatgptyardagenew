import React from 'react';
import styles from './ClubSelector.module.css';

interface ClubData {
  name: string;
  loft: number;         // degrees
  avgDistance: number;  // yards
  avgSpinRate: number; // rpm
  launchAngle: number; // degrees
}

interface ClubSelectorProps {
  selectedClub?: string;
  clubData?: ClubData[];
  onClubSelect?: (clubName: string) => void;
}

const defaultClubs: ClubData[] = [
  { name: 'driver', loft: 10.5, avgDistance: 230, avgSpinRate: 2700, launchAngle: 10.9 },
  { name: '3-wood', loft: 15, avgDistance: 215, avgSpinRate: 3000, launchAngle: 12.5 },
  { name: '5-wood', loft: 18, avgDistance: 200, avgSpinRate: 3300, launchAngle: 14 },
  { name: '4-iron', loft: 24, avgDistance: 185, avgSpinRate: 4000, launchAngle: 15 },
  { name: '5-iron', loft: 27, avgDistance: 175, avgSpinRate: 4500, launchAngle: 16 },
  { name: '6-iron', loft: 30, avgDistance: 165, avgSpinRate: 5000, launchAngle: 17 },
  { name: '7-iron', loft: 34, avgDistance: 155, avgSpinRate: 5500, launchAngle: 18.5 },
  { name: '8-iron', loft: 38, avgDistance: 145, avgSpinRate: 6000, launchAngle: 20 },
  { name: '9-iron', loft: 42, avgDistance: 135, avgSpinRate: 6700, launchAngle: 22 },
  { name: 'pw', loft: 46, avgDistance: 120, avgSpinRate: 7500, launchAngle: 24 },
  { name: 'gap-wedge', loft: 50, avgDistance: 110, avgSpinRate: 8000, launchAngle: 26 },
  { name: 'sand-wedge', loft: 54, avgDistance: 100, avgSpinRate: 8500, launchAngle: 28 },
  { name: 'lob-wedge', loft: 58, avgDistance: 90, avgSpinRate: 9000, launchAngle: 30 }
];

const getClubDisplayName = (name: string): string => {
  const displayNames: { [key: string]: string } = {
    'pw': 'Pitching Wedge',
    'gap-wedge': 'Gap Wedge',
    'sand-wedge': 'Sand Wedge',
    'lob-wedge': 'Lob Wedge'
  };
  
  if (name in displayNames) {
    return displayNames[name];
  }
  
  return name.split('-')
    .map(word => word.charAt(0).toUpperCase() + word.slice(1))
    .join(' ');
};

const getStatTooltip = (stat: string): string => {
  const tooltips: { [key: string]: string } = {
    loft: 'Club loft angle affects ball trajectory and spin',
    avgDistance: 'Average carry distance with this club',
    avgSpinRate: 'Average backspin rate affects ball flight and control',
    launchAngle: 'Typical launch angle for optimal trajectory'
  };
  return tooltips[stat] || '';
};

const ClubSelector: React.FC<ClubSelectorProps> = ({
  selectedClub = 'driver',
  clubData = defaultClubs,
  onClubSelect
}) => {
  const selectedClubData = clubData.find(club => club.name === selectedClub);
  const clubImageUrl = `/images/clubs/${selectedClub}.png`;

  return (
    <div className={styles['club-selector']}>
      <div className={styles['selector-header']}>
        <h2 className={styles['selector-title']}>Club Selection</h2>
      </div>

      <div className={styles['club-select-container']}>
        <select 
          value={selectedClub}
          onChange={(e) => onClubSelect?.(e.target.value)}
          className={styles['club-select']}
          data-testid="club-selector"
          aria-label="Select club"
        >
          {clubData.map(club => (
            <option key={club.name} value={club.name}>
              {getClubDisplayName(club.name)}
            </option>
          ))}
        </select>
        <span className={styles['select-arrow']}>▼</span>
      </div>

      {selectedClubData && (
        <>
          <div 
            className={styles['club-image']}
            style={{ backgroundImage: `url(${clubImageUrl})` }}
            role="img"
            aria-label={`Image of ${getClubDisplayName(selectedClubData.name)}`}
          />
          <div className={styles['club-stats']}>
            <div className={styles.stat}>
              <div className={styles.tooltip}>{getStatTooltip('loft')}</div>
              <span className={styles['stat-label']}>Loft</span>
              <span className={styles['stat-value']}>{selectedClubData.loft}°</span>
            </div>
            <div className={styles.stat}>
              <div className={styles.tooltip}>{getStatTooltip('avgDistance')}</div>
              <span className={styles['stat-label']}>Avg Distance</span>
              <span className={styles['stat-value']}>{selectedClubData.avgDistance} yards</span>
            </div>
            <div className={styles.stat}>
              <div className={styles.tooltip}>{getStatTooltip('avgSpinRate')}</div>
              <span className={styles['stat-label']}>Avg Spin Rate</span>
              <span className={styles['stat-value']}>
                {selectedClubData.avgSpinRate.toLocaleString()} rpm
              </span>
            </div>
            <div className={styles.stat}>
              <div className={styles.tooltip}>{getStatTooltip('launchAngle')}</div>
              <span className={styles['stat-label']}>Launch Angle</span>
              <span className={styles['stat-value']}>{selectedClubData.launchAngle}°</span>
            </div>
          </div>
        </>
      )}
    </div>
  );
};

export default ClubSelector;
