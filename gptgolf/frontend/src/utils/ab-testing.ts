import { analyticsSystem } from './analytics-system';

interface Experiment {
    id: string;
    name: string;
    description: string;
    variants: Variant[];
    startDate: number;
    endDate?: number;
    isActive: boolean;
    targetAudience?: {
        userTypes?: string[];
        percentage?: number;
    };
}

interface Variant {
    id: string;
    name: string;
    weight: number; // Percentage of users to assign (0-100)
}

interface ExperimentAssignment {
    experimentId: string;
    variantId: string;
    userId: string;
    timestamp: number;
}

class ABTestingSystem {
    private static instance: ABTestingSystem;
    private experiments: Map<string, Experiment> = new Map();
    private assignments: Map<string, ExperimentAssignment> = new Map();
    private readonly storageKey = 'golf_app_ab_tests';

    private constructor() {
        this.loadState();
        this.setupAnalyticsTracking();
    }

    public static getInstance(): ABTestingSystem {
        if (!ABTestingSystem.instance) {
            ABTestingSystem.instance = new ABTestingSystem();
        }
        return ABTestingSystem.instance;
    }

    private loadState(): void {
        const storedState = localStorage.getItem(this.storageKey);
        if (storedState) {
            try {
                const { experiments, assignments } = JSON.parse(storedState);
                this.experiments = new Map(experiments);
                this.assignments = new Map(assignments);
            } catch (error) {
                console.error('Error loading A/B testing state:', error);
            }
        }
    }

    private saveState(): void {
        try {
            const state = {
                experiments: Array.from(this.experiments.entries()),
                assignments: Array.from(this.assignments.entries())
            };
            localStorage.setItem(this.storageKey, JSON.stringify(state));
        } catch (error) {
            console.error('Error saving A/B testing state:', error);
        }
    }

    private setupAnalyticsTracking(): void {
        // Track experiment views and interactions
        analyticsSystem.onEvent((event) => {
            if (event.userId) {
                const userAssignments = this.getUserAssignments(event.userId);
                if (userAssignments.length > 0) {
                    // Add experiment data to analytics event metadata
                    event.metadata = {
                        ...event.metadata,
                        experiments: userAssignments
                    };
                }
            }
        });
    }

    public createExperiment(experiment: Omit<Experiment, 'id' | 'startDate' | 'isActive'>): string {
        const id = crypto.randomUUID();
        const newExperiment: Experiment = {
            id,
            startDate: Date.now(),
            isActive: true,
            ...experiment
        };

        // Validate variant weights sum to 100
        const totalWeight = newExperiment.variants.reduce((sum, variant) => sum + variant.weight, 0);
        if (Math.abs(totalWeight - 100) > 0.01) {
            throw new Error('Variant weights must sum to 100');
        }

        this.experiments.set(id, newExperiment);
        this.saveState();
        return id;
    }

    public getExperiment(experimentId: string): Experiment | undefined {
        return this.experiments.get(experimentId);
    }

    public getAllExperiments(): Experiment[] {
        return Array.from(this.experiments.values());
    }

    public getActiveExperiments(): Experiment[] {
        return Array.from(this.experiments.values()).filter(exp => exp.isActive);
    }

    public assignUserToVariant(experimentId: string, userId: string): string | null {
        const experiment = this.experiments.get(experimentId);
        if (!experiment || !experiment.isActive) {
            return null;
        }

        // Check if user is already assigned
        const existingAssignment = this.assignments.get(`${experimentId}:${userId}`);
        if (existingAssignment) {
            return existingAssignment.variantId;
        }

        // Check target audience criteria
        if (experiment.targetAudience?.percentage) {
            const userHash = this.hashString(userId);
            const normalizedHash = userHash / Math.pow(2, 32);
            if (normalizedHash > experiment.targetAudience.percentage / 100) {
                return null;
            }
        }

        // Randomly assign variant based on weights
        const random = Math.random() * 100;
        let cumulativeWeight = 0;
        let selectedVariant: Variant | null = null;

        for (const variant of experiment.variants) {
            cumulativeWeight += variant.weight;
            if (random <= cumulativeWeight) {
                selectedVariant = variant;
                break;
            }
        }

        if (!selectedVariant) {
            return null;
        }

        const assignment: ExperimentAssignment = {
            experimentId,
            variantId: selectedVariant.id,
            userId,
            timestamp: Date.now()
        };

        this.assignments.set(`${experimentId}:${userId}`, assignment);
        this.saveState();

        // Track assignment in analytics
        analyticsSystem.trackEvent({
            type: 'experiment_assignment',
            category: 'business',
            action: 'variant_assigned',
            label: experiment.name,
            metadata: {
                experimentId,
                variantId: selectedVariant.id
            }
        });

        return selectedVariant.id;
    }

    public getUserVariant(experimentId: string, userId: string): string | null {
        const assignment = this.assignments.get(`${experimentId}:${userId}`);
        return assignment?.variantId || null;
    }

    public getUserAssignments(userId: string): ExperimentAssignment[] {
        return Array.from(this.assignments.values())
            .filter(assignment => assignment.userId === userId);
    }

    public endExperiment(experimentId: string): void {
        const experiment = this.experiments.get(experimentId);
        if (experiment) {
            experiment.isActive = false;
            experiment.endDate = Date.now();
            this.experiments.set(experimentId, experiment);
            this.saveState();
        }
    }

    public getExperimentResults(experimentId: string): {
        totalParticipants: number;
        variantDistribution: Record<string, number>;
        conversionRates: Record<string, number>;
    } {
        const assignments = Array.from(this.assignments.values())
            .filter(a => a.experimentId === experimentId);

        const results = {
            totalParticipants: assignments.length,
            variantDistribution: {} as Record<string, number>,
            conversionRates: {} as Record<string, number>
        };

        // Calculate variant distribution
        assignments.forEach(assignment => {
            results.variantDistribution[assignment.variantId] = 
                (results.variantDistribution[assignment.variantId] || 0) + 1;
        });

        // Calculate conversion rates (using analytics data)
        const experiment = this.experiments.get(experimentId);
        if (experiment) {
            experiment.variants.forEach(variant => {
                const variantAssignments = assignments.filter(a => a.variantId === variant.id);
                const conversions = analyticsSystem.getEvents()
                    .filter(event => 
                        event.type === 'conversion' &&
                        event.metadata?.experimentId === experimentId &&
                        event.metadata?.variantId === variant.id
                    );
                
                results.conversionRates[variant.id] = 
                    variantAssignments.length > 0 
                        ? conversions.length / variantAssignments.length 
                        : 0;
            });
        }

        return results;
    }

    private hashString(str: string): number {
        let hash = 0;
        for (let i = 0; i < str.length; i++) {
            const char = str.charCodeAt(i);
            hash = ((hash << 5) - hash) + char;
            hash = hash & hash; // Convert to 32-bit integer
        }
        return Math.abs(hash);
    }
}

export const abTestingSystem = ABTestingSystem.getInstance();
export type { Experiment, Variant, ExperimentAssignment };
