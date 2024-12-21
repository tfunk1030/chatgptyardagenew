interface ErrorLog {
    timestamp: number;
    type: string;
    message: string;
    component?: string;
    stack?: string;
    metadata?: Record<string, any>;
}

class ErrorTracker {
    private static instance: ErrorTracker;
    private errors: ErrorLog[] = [];
    private readonly maxErrors = 100;
    private errorCallbacks: ((error: ErrorLog) => void)[] = [];

    private constructor() {
        this.setupGlobalErrorHandling();
    }

    public static getInstance(): ErrorTracker {
        if (!ErrorTracker.instance) {
            ErrorTracker.instance = new ErrorTracker();
        }
        return ErrorTracker.instance;
    }

    private setupGlobalErrorHandling(): void {
        window.onerror = (message, source, lineno, colno, error) => {
            this.logError({
                type: 'uncaught',
                message: message.toString(),
                stack: error?.stack,
                metadata: {
                    source,
                    lineno,
                    colno
                }
            });
            return false; // Let default error handling continue
        };

        window.addEventListener('unhandledrejection', (event) => {
            this.logError({
                type: 'promise',
                message: event.reason?.message || 'Unhandled Promise Rejection',
                stack: event.reason?.stack,
                metadata: {
                    reason: event.reason
                }
            });
        });
    }

    public logError(error: Omit<ErrorLog, 'timestamp'>): void {
        const errorLog: ErrorLog = {
            timestamp: Date.now(),
            ...error
        };

        this.errors.unshift(errorLog);
        
        // Keep error log size manageable
        if (this.errors.length > this.maxErrors) {
            this.errors.pop();
        }

        // Notify subscribers
        this.errorCallbacks.forEach(callback => callback(errorLog));

        // Log to console in development
        if (process.env.NODE_ENV === 'development') {
            console.error('Error tracked:', errorLog);
        }
    }

    public onError(callback: (error: ErrorLog) => void): () => void {
        this.errorCallbacks.push(callback);
        return () => {
            this.errorCallbacks = this.errorCallbacks.filter(cb => cb !== callback);
        };
    }

    public getErrors(): ErrorLog[] {
        return [...this.errors];
    }

    public clearErrors(): void {
        this.errors = [];
    }

    public getErrorStats(): {
        total: number;
        byType: Record<string, number>;
        byComponent: Record<string, number>;
    } {
        const stats = {
            total: this.errors.length,
            byType: {} as Record<string, number>,
            byComponent: {} as Record<string, number>
        };

        this.errors.forEach(error => {
            // Count by type
            stats.byType[error.type] = (stats.byType[error.type] || 0) + 1;

            // Count by component if available
            if (error.component) {
                stats.byComponent[error.component] = (stats.byComponent[error.component] || 0) + 1;
            }
        });

        return stats;
    }

    public isErrorTrendingUp(timeWindow: number = 3600000): boolean {
        const now = Date.now();
        const recentErrors = this.errors.filter(
            error => error.timestamp > now - timeWindow
        );

        const halfWindow = timeWindow / 2;
        const firstHalf = recentErrors.filter(
            error => error.timestamp <= now - halfWindow
        ).length;
        const secondHalf = recentErrors.filter(
            error => error.timestamp > now - halfWindow
        ).length;

        return secondHalf > firstHalf * 1.5; // 50% increase threshold
    }

    public getErrorFrequency(timeWindow: number = 3600000): number {
        const now = Date.now();
        const recentErrors = this.errors.filter(
            error => error.timestamp > now - timeWindow
        );
        return recentErrors.length / (timeWindow / 1000); // Errors per second
    }
}

export const errorTracker = ErrorTracker.getInstance();
export type { ErrorLog };
