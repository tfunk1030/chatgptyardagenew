/// <reference types="jest" />
/// <reference types="@testing-library/react" />
/// <reference types="@testing-library/jest-dom" />
/// <reference types="@testing-library/react-hooks" />

declare namespace jest {
    interface Matchers<R> {
        toBeInTheDocument(): R;
        toHaveStyle(style: { [key: string]: any }): R;
        toBeVisible(): R;
        toBeDisabled(): R;
        toHaveClass(className: string): R;
        toHaveAttribute(attr: string, value?: string): R;
        toHaveTextContent(text: string | RegExp): R;
        toBeDeepCloseTo(expected: number | number[] | object, precision?: number): R;
        toMatchShapeOf(expected: any): R;
    }
}

declare module '@tensorflow/tfjs' {
    export interface Tensor {
        dispose(): void;
        data(): Promise<Float32Array>;
    }

    export interface LayersModel {
        predict(inputs: Tensor): Tensor;
        dispose(): void;
    }

    export interface Sequential extends LayersModel {
        add(layer: Layer): void;
        compile(config: CompileConfig): void;
        fit(x: Tensor, y: Tensor, config?: FitConfig): Promise<History>;
    }

    export interface Layer {}
    export interface CompileConfig {}
    export interface FitConfig {}
    export interface History {}

    export function loadLayersModel(path: string): Promise<LayersModel>;
    export function tensor(values: number[], shape?: number[]): Tensor;
    export function tensor2d(values: number[][], shape?: [number, number]): Tensor;
    export function ones(shape: number[]): Tensor;
    export function dispose(): void;
    export function ready(): Promise<void>;
    export function tidy<T>(fn: () => T): T;

    export namespace browser {
        export function fromPixels(pixels: ImageData | HTMLImageElement | HTMLCanvasElement | HTMLVideoElement): Tensor;
    }

    export function memory(): {
        numBytes: number;
        numTensors: number;
        numDataBuffers: number;
        unreliable: boolean;
    };
}

declare module '@testing-library/react-hooks' {
    export function renderHook<P, R>(callback: (props: P) => R): {
        result: { current: R };
        waitForNextUpdate: () => Promise<void>;
        rerender: (props?: P) => void;
        unmount: () => void;
    };

    export function act(callback: () => void | Promise<void>): Promise<void>;
}

declare module 'jest-canvas-mock';
declare module 'jest-webgl-canvas-mock';

declare global {
    interface Window {
        WebGLRenderingContext: WebGLRenderingContext;
        requestAnimationFrame(callback: FrameRequestCallback): number;
        cancelAnimationFrame(handle: number): void;
    }

    interface HTMLCanvasElement {
        getContext(contextId: '2d'): CanvasRenderingContext2D | null;
        getContext(contextId: 'webgl'): WebGLRenderingContext | null;
        getContext(contextId: 'webgl2'): WebGL2RenderingContext | null;
        getContext(contextId: 'bitmaprenderer'): ImageBitmapRenderingContext | null;
    }

    class ResizeObserver {
        constructor(callback: ResizeObserverCallback);
        observe(target: Element, options?: ResizeObserverOptions): void;
        unobserve(target: Element): void;
        disconnect(): void;
    }

    interface ResizeObserverCallback {
        (entries: ResizeObserverEntry[], observer: ResizeObserver): void;
    }

    interface ResizeObserverEntry {
        readonly target: Element;
        readonly contentRect: DOMRectReadOnly;
        readonly borderBoxSize: ReadonlyArray<ResizeObserverSize>;
        readonly contentBoxSize: ReadonlyArray<ResizeObserverSize>;
        readonly devicePixelContentBoxSize: ReadonlyArray<ResizeObserverSize>;
    }

    interface ResizeObserverSize {
        readonly inlineSize: number;
        readonly blockSize: number;
    }

    interface ResizeObserverOptions {
        box?: 'content-box' | 'border-box' | 'device-pixel-content-box';
    }
}

export {};
