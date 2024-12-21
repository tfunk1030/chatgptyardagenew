interface FeedbackItem {
    id: string;
    timestamp: number;
    type: 'bug' | 'feature' | 'improvement' | 'general';
    category: 'physics' | 'ui' | 'performance' | 'other';
    title: string;
    description: string;
    userId?: string;
    metadata?: Record<string, any>;
    status: 'new' | 'in-review' | 'accepted' | 'rejected' | 'implemented';
    priority: 'low' | 'medium' | 'high' | 'critical';
    votes: number;
}

class FeedbackSystem {
    private static instance: FeedbackSystem;
    private feedback: FeedbackItem[] = [];
    private feedbackCallbacks: ((feedback: FeedbackItem) => void)[] = [];
    private storageKey = 'golf_app_feedback';

    private constructor() {
        this.loadFeedback();
    }

    public static getInstance(): FeedbackSystem {
        if (!FeedbackSystem.instance) {
            FeedbackSystem.instance = new FeedbackSystem();
        }
        return FeedbackSystem.instance;
    }

    private loadFeedback(): void {
        const storedFeedback = localStorage.getItem(this.storageKey);
        if (storedFeedback) {
            try {
                this.feedback = JSON.parse(storedFeedback);
            } catch (error) {
                console.error('Error loading feedback:', error);
                this.feedback = [];
            }
        }
    }

    private saveFeedback(): void {
        try {
            localStorage.setItem(this.storageKey, JSON.stringify(this.feedback));
        } catch (error) {
            console.error('Error saving feedback:', error);
        }
    }

    public submitFeedback(feedback: Omit<FeedbackItem, 'id' | 'timestamp' | 'votes' | 'status'>): string {
        const newFeedback: FeedbackItem = {
            id: crypto.randomUUID(),
            timestamp: Date.now(),
            votes: 0,
            status: 'new',
            ...feedback
        };

        this.feedback.unshift(newFeedback);
        this.saveFeedback();

        // Notify subscribers
        this.feedbackCallbacks.forEach(callback => callback(newFeedback));

        return newFeedback.id;
    }

    public onFeedback(callback: (feedback: FeedbackItem) => void): () => void {
        this.feedbackCallbacks.push(callback);
        return () => {
            this.feedbackCallbacks = this.feedbackCallbacks.filter(cb => cb !== callback);
        };
    }

    public getFeedback(): FeedbackItem[] {
        return [...this.feedback];
    }

    public getFeedbackById(id: string): FeedbackItem | undefined {
        return this.feedback.find(item => item.id === id);
    }

    public updateFeedbackStatus(id: string, status: FeedbackItem['status']): boolean {
        const index = this.feedback.findIndex(item => item.id === id);
        if (index !== -1) {
            this.feedback[index] = {
                ...this.feedback[index],
                status
            };
            this.saveFeedback();
            return true;
        }
        return false;
    }

    public voteFeedback(id: string, increment: boolean = true): boolean {
        const index = this.feedback.findIndex(item => item.id === id);
        if (index !== -1) {
            this.feedback[index] = {
                ...this.feedback[index],
                votes: this.feedback[index].votes + (increment ? 1 : -1)
            };
            this.saveFeedback();
            return true;
        }
        return false;
    }

    public getFeedbackStats(): {
        total: number;
        byType: Record<FeedbackItem['type'], number>;
        byCategory: Record<FeedbackItem['category'], number>;
        byStatus: Record<FeedbackItem['status'], number>;
        byPriority: Record<FeedbackItem['priority'], number>;
    } {
        const stats = {
            total: this.feedback.length,
            byType: {} as Record<FeedbackItem['type'], number>,
            byCategory: {} as Record<FeedbackItem['category'], number>,
            byStatus: {} as Record<FeedbackItem['status'], number>,
            byPriority: {} as Record<FeedbackItem['priority'], number>
        };

        this.feedback.forEach(item => {
            stats.byType[item.type] = (stats.byType[item.type] || 0) + 1;
            stats.byCategory[item.category] = (stats.byCategory[item.category] || 0) + 1;
            stats.byStatus[item.status] = (stats.byStatus[item.status] || 0) + 1;
            stats.byPriority[item.priority] = (stats.byPriority[item.priority] || 0) + 1;
        });

        return stats;
    }

    public getTopFeedback(limit: number = 10): FeedbackItem[] {
        return [...this.feedback]
            .sort((a, b) => b.votes - a.votes)
            .slice(0, limit);
    }

    public searchFeedback(query: string): FeedbackItem[] {
        const lowercaseQuery = query.toLowerCase();
        return this.feedback.filter(item =>
            item.title.toLowerCase().includes(lowercaseQuery) ||
            item.description.toLowerCase().includes(lowercaseQuery)
        );
    }

    public getFeedbackByCategory(category: FeedbackItem['category']): FeedbackItem[] {
        return this.feedback.filter(item => item.category === category);
    }

    public clearFeedback(): void {
        this.feedback = [];
        this.saveFeedback();
    }
}

export const feedbackSystem = FeedbackSystem.getInstance();
export type { FeedbackItem };
