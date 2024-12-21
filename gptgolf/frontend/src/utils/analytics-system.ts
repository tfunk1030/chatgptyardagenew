interface AnalyticsEvent {
    id: string;
    timestamp: number;
    type: string;
    category: 'interaction' | 'performance' | 'error' | 'business';
    action: string;
    label?: string;
    value?: number;
    metadata?: Record<string, any>;
    sessionId: string;
    userId?: string;
}

interface UserSession {
    id: string;
    startTime: number;
    endTime?: number;
    events: AnalyticsEvent[];
    metadata?: Record<string, any>;
}

class AnalyticsSystem {
    private static instance: AnalyticsSystem;
    private events: AnalyticsEvent[] = [];
    private currentSession: UserSession;
    private readonly storageKey = 'golf_app_analytics';
    private analyticsCallbacks: ((event: AnalyticsEvent) => void)[] = [];
    private sessionTimeout: number = 30 * 60 * 1000; // 30 minutes
    private sessionCheckInterval?: number;

    private constructor() {
        this.currentSession = this.startNewSession();
        this.loadEvents();
        this.setupSessionTracking();
    }

    public static getInstance(): AnalyticsSystem {
        if (!AnalyticsSystem.instance) {
            AnalyticsSystem.instance = new AnalyticsSystem();
        }
        return AnalyticsSystem.instance;
    }

    private startNewSession(): UserSession {
        return {
            id: crypto.randomUUID(),
            startTime: Date.now(),
            events: []
        };
    }

    private setupSessionTracking(): void {
        // Check session status periodically
        this.sessionCheckInterval = window.setInterval(() => {
            const now = Date.now();
            const lastEventTime = this.currentSession.events.length > 0
                ? this.currentSession.events[this.currentSession.events.length - 1].timestamp
                : this.currentSession.startTime;

            if (now - lastEventTime > this.sessionTimeout) {
                this.endCurrentSession();
                this.currentSession = this.startNewSession();
            }
        }, 60000) as unknown as number; // Check every minute

        // Track page visibility
        document.addEventListener('visibilitychange', () => {
            if (document.hidden) {
                this.trackEvent({
                    type: 'visibility',
                    category: 'interaction',
                    action: 'page_hidden'
                });
            } else {
                this.trackEvent({
                    type: 'visibility',
                    category: 'interaction',
                    action: 'page_visible'
                });
            }
        });

        // Track page unload
        window.addEventListener('beforeunload', () => {
            this.endCurrentSession();
        });
    }

    private endCurrentSession(): void {
        this.currentSession.endTime = Date.now();
        this.saveEvents();
    }

    private loadEvents(): void {
        const storedEvents = localStorage.getItem(this.storageKey);
        if (storedEvents) {
            try {
                this.events = JSON.parse(storedEvents);
            } catch (error) {
                console.error('Error loading analytics events:', error);
                this.events = [];
            }
        }
    }

    private saveEvents(): void {
        try {
            localStorage.setItem(this.storageKey, JSON.stringify(this.events));
        } catch (error) {
            console.error('Error saving analytics events:', error);
        }
    }

    public trackEvent(eventData: Omit<AnalyticsEvent, 'id' | 'timestamp' | 'sessionId'>): void {
        const event: AnalyticsEvent = {
            id: crypto.randomUUID(),
            timestamp: Date.now(),
            sessionId: this.currentSession.id,
            ...eventData
        };

        this.events.push(event);
        this.currentSession.events.push(event);
        this.saveEvents();

        // Notify subscribers
        this.analyticsCallbacks.forEach(callback => callback(event));
    }

    public onEvent(callback: (event: AnalyticsEvent) => void): () => void {
        this.analyticsCallbacks.push(callback);
        return () => {
            this.analyticsCallbacks = this.analyticsCallbacks.filter(cb => cb !== callback);
        };
    }

    public getEvents(startTime?: number, endTime?: number): AnalyticsEvent[] {
        let filteredEvents = [...this.events];
        
        if (startTime) {
            filteredEvents = filteredEvents.filter(event => event.timestamp >= startTime);
        }
        
        if (endTime) {
            filteredEvents = filteredEvents.filter(event => event.timestamp <= endTime);
        }
        
        return filteredEvents;
    }

    public getEventsByType(type: string): AnalyticsEvent[] {
        return this.events.filter(event => event.type === type);
    }

    public getEventsByCategory(category: AnalyticsEvent['category']): AnalyticsEvent[] {
        return this.events.filter(event => event.category === category);
    }

    public getAnalyticsSummary(timeWindow: number = 24 * 60 * 60 * 1000): {
        totalEvents: number;
        uniqueUsers: number;
        eventsByCategory: Record<string, number>;
        eventsByType: Record<string, number>;
        averageSessionDuration: number;
        activeUsers: number;
    } {
        const now = Date.now();
        const recentEvents = this.events.filter(event => event.timestamp > now - timeWindow);

        const uniqueUsers = new Set(recentEvents.filter(e => e.userId).map(e => e.userId)).size;
        const uniqueSessions = new Set(recentEvents.map(e => e.sessionId)).size;

        const summary = {
            totalEvents: recentEvents.length,
            uniqueUsers,
            eventsByCategory: {} as Record<string, number>,
            eventsByType: {} as Record<string, number>,
            averageSessionDuration: 0,
            activeUsers: uniqueSessions
        };

        recentEvents.forEach(event => {
            summary.eventsByCategory[event.category] = (summary.eventsByCategory[event.category] || 0) + 1;
            summary.eventsByType[event.type] = (summary.eventsByType[event.type] || 0) + 1;
        });

        return summary;
    }

    public clearEvents(): void {
        this.events = [];
        this.saveEvents();
    }

    public destroy(): void {
        if (this.sessionCheckInterval) {
            clearInterval(this.sessionCheckInterval);
        }
    }
}

export const analyticsSystem = AnalyticsSystem.getInstance();
export type { AnalyticsEvent, UserSession };
