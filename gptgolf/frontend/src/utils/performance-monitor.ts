    interface PerformanceMetrics {
    renderTime: number;
    updateTime: number;
    interactionTime: number;
    memoryUsage?: number;
}

class PerformanceMonitor {
    private static instance: PerformanceMonitor;
    private metrics: PerformanceMetrics = {
        renderTime: 0,
        updateTime: 0,
        interactionTime: 0
    };

    private constructor() {
        // Initialize performance monitoring
        if (window.performance && window.performance.memory) {
            this.startMemoryMonitoring();
        }
    }

    public static getInstance(): PerformanceMonitor {
        if (!PerformanceMonitor.instance) {
            PerformanceMonitor.instance = new PerformanceMonitor();
        }
        return PerformanceMonitor.instance;
    }

    public startMeasure(name: string): number {
        return performance.now();
    }

    public endMeasure(name: string, startTime: number): number {
        const endTime = performance.now();
        const duration = endTime - startTime;

        switch (name) {
            case 'render':
                this.metrics.renderTime = duration;
                break;
            case 'update':
                this.metrics.updateTime = duration;
                break;
            case 'interaction':
                this.metrics.interactionTime = duration;
                break;
        }

        return duration;
    }

    public getMetrics(): PerformanceMetrics {
        return { ...this.metrics };
    }

    private startMemoryMonitoring(): void {
        setInterval(() => {
            if (window.performance && window.performance.memory) {
                this.metrics.memoryUsage = window.performance.memory.usedJSHeapSize / (1024 * 1024); // MB
            }
        }, 1000);
    }

    public checkPerformanceBudget(metrics: Partial<PerformanceMetrics>): boolean {
        const budgets = {
            renderTime: 100,    // 100ms
            updateTime: 50,     // 50ms
            interactionTime: 50 // 50ms
        };

        return Object.entries(metrics).every(([key, value]) => {
            const budget = budgets[key as keyof typeof budgets];
            return budget ? value <= budget : true;
        });
    }

    public logPerformanceViolation(component: string, metric: string, value: number, budget: number): void {
        console.warn(
            `Performance budget exceeded in ${component}:`,
            `\n${metric}: ${value.toFixed(2)}ms`,
            `\nBudget: ${budget}ms`,
            `\nExcess: ${(value - budget).toFixed(2)}ms`
        );
    }
}

// Add type declaration for window.performance.memory
declare global {
    interface Performance {
        memory?: {
            usedJSHeapSize: number;
            totalJSHeapSize: number;
            jsHeapSizeLimit: number;
        };
    }
}

export const performanceMonitor = PerformanceMonitor.getInstance();
export type { PerformanceMetrics };
