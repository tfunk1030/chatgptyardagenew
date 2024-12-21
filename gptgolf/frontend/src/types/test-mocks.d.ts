/// <reference types="jest" />

declare global {
    interface Window {
        WebGLRenderingContext: WebGLRenderingContext;
    }

    // Override the default getContext method
    interface HTMLCanvasElement {
        getContext(contextId: '2d', options?: CanvasRenderingContext2DSettings): CanvasRenderingContext2D | null;
        getContext(contextId: 'webgl', options?: WebGLContextAttributes): WebGLRenderingContext | null;
        getContext(contextId: 'webgl2', options?: WebGLContextAttributes): WebGL2RenderingContext | null;
        getContext(contextId: 'bitmaprenderer', options?: ImageBitmapRenderingContextSettings): ImageBitmapRenderingContext | null;
        // Add a catch-all for other context types
        getContext(contextId: string, options?: any): RenderingContext | null;
    }

    // Add custom matchers
    namespace jest {
        interface Matchers<R> {
            toBeDeepCloseTo: (expected: number | number[] | object, precision?: number) => R;
            toMatchShapeOf: (expected: any) => R;
        }
    }

    // Mock TensorFlow.js types
    namespace tf {
        interface Tensor {
            dispose(): void;
            data(): Promise<Float32Array>;
        }

        interface LayersModel {
            predict(inputs: Tensor): Tensor;
            dispose(): void;
        }

        interface Sequential extends LayersModel {
            add(layer: Layer): void;
            compile(config: CompileConfig): void;
            fit(x: Tensor, y: Tensor, config?: FitConfig): Promise<History>;
        }

        interface Layer {}
        interface CompileConfig {}
        interface FitConfig {}
        interface History {}

        function loadLayersModel(path: string): Promise<LayersModel>;
        function tensor(values: number[], shape?: number[]): Tensor;
        function tensor2d(values: number[][], shape?: [number, number]): Tensor;
        function ones(shape: number[]): Tensor;
    }

    // Add requestAnimationFrame types
    interface Window {
        requestAnimationFrame(callback: FrameRequestCallback): number;
        cancelAnimationFrame(handle: number): void;
    }

    // Add ResizeObserver types
    class ResizeObserver {
        constructor(callback: ResizeObserverCallback);
        observe(target: Element, options?: ResizeObserverOptions): void;
        unobserve(target: Element): void;
        disconnect(): void;
    }
}

// Extend NodeJS.Timer to include number
declare module 'timers' {
    interface Timer extends Number {}
}

// Make setTimeout return a number
declare function setTimeout(callback: (...args: any[]) => void, ms: number, ...args: any[]): number;

export {};
