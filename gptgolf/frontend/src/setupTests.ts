// jest-dom adds custom jest matchers for asserting on DOM nodes.
import '@testing-library/jest-dom';
import 'jest-canvas-mock';
import '@tensorflow/tfjs';

// Mock WebGL context
const mockWebGLContext: Partial<WebGLRenderingContext> = {
    canvas: document.createElement('canvas'),
    drawingBufferWidth: 0,
    drawingBufferHeight: 0,
    getExtension: jest.fn(),
    getParameter: jest.fn(),
    getShaderPrecisionFormat: jest.fn(() => ({
        precision: 1,
        rangeMin: 1,
        rangeMax: 1
    })),
    createBuffer: jest.fn(),
    bindBuffer: jest.fn(),
    bufferData: jest.fn(),
    enable: jest.fn(),
    disable: jest.fn(),
    blendFunc: jest.fn(),
    clear: jest.fn(),
    viewport: jest.fn(),
    createShader: jest.fn(),
    shaderSource: jest.fn(),
    compileShader: jest.fn(),
    getShaderParameter: jest.fn(() => true),
    createProgram: jest.fn(),
    attachShader: jest.fn(),
    linkProgram: jest.fn(),
    getProgramParameter: jest.fn(() => true),
    useProgram: jest.fn(),
    createTexture: jest.fn(),
    bindTexture: jest.fn(),
    texImage2D: jest.fn(),
    texParameteri: jest.fn(),
    activeTexture: jest.fn(),
    getUniformLocation: jest.fn(),
    uniform1i: jest.fn(),
    uniform1f: jest.fn(),
    uniform2f: jest.fn(),
    uniform3f: jest.fn(),
    uniform4f: jest.fn(),
    uniformMatrix4fv: jest.fn(),
    getAttribLocation: jest.fn(),
    vertexAttribPointer: jest.fn(),
    enableVertexAttribArray: jest.fn(),
    drawArrays: jest.fn(),
    drawElements: jest.fn(),
    deleteShader: jest.fn(),
    deleteProgram: jest.fn(),
    deleteTexture: jest.fn(),
    deleteBuffer: jest.fn(),
    clearColor: jest.fn(),
    clearDepth: jest.fn(),
    DEPTH_TEST: 0x0B71,
    BLEND: 0x0BE2,
    ARRAY_BUFFER: 0x8892,
    ELEMENT_ARRAY_BUFFER: 0x8893,
    STATIC_DRAW: 0x88E4,
    FLOAT: 0x1406,
    TRIANGLES: 0x0004,
    UNSIGNED_SHORT: 0x1403,
    TEXTURE_2D: 0x0DE1,
    TEXTURE0: 0x84C0,
    TEXTURE_MIN_FILTER: 0x2801,
    TEXTURE_MAG_FILTER: 0x2800,
    LINEAR: 0x2601,
    NEAREST: 0x2600,
    RGBA: 0x1908,
    UNSIGNED_BYTE: 0x1401,
    VERTEX_SHADER: 0x8B31,
    FRAGMENT_SHADER: 0x8B30,
    COMPILE_STATUS: 0x8B81,
    LINK_STATUS: 0x8B82,
    COLOR_BUFFER_BIT: 0x4000,
    DEPTH_BUFFER_BIT: 0x0100,
    SRC_ALPHA: 0x0302,
    ONE_MINUS_SRC_ALPHA: 0x0303
} as unknown as WebGLRenderingContext;

// Override getContext to return our mock contexts
const originalGetContext = HTMLCanvasElement.prototype.getContext;
HTMLCanvasElement.prototype.getContext = function(
    this: HTMLCanvasElement,
    contextId: '2d' | 'webgl' | 'webgl2' | 'bitmaprenderer',
    options?: any
): WebGLRenderingContext | CanvasRenderingContext2D | null {
    if (contextId === 'webgl' || contextId === 'webgl2') {
        return mockWebGLContext as WebGLRenderingContext;
    }
    return originalGetContext.call(this, contextId, options);
};

// Mock requestAnimationFrame
const originalRAF = global.requestAnimationFrame;
global.requestAnimationFrame = function(callback: FrameRequestCallback): number {
    return Number(setTimeout(() => callback(Date.now()), 16));
};

global.cancelAnimationFrame = function(handle: number): void {
    clearTimeout(handle);
};

// Mock ResizeObserver
global.ResizeObserver = class ResizeObserver {
    constructor(callback: ResizeObserverCallback) {}
    observe(target: Element, options?: ResizeObserverOptions): void {}
    unobserve(target: Element): void {}
    disconnect(): void {}
};

// Mock window.URL.createObjectURL
global.URL.createObjectURL = jest.fn();

// Mock TensorFlow.js environment
jest.mock('@tensorflow/tfjs', () => ({
    ready: jest.fn().mockResolvedValue(true),
    setBackend: jest.fn(),
    browser: {
        fromPixels: jest.fn()
    },
    tensor: jest.fn(),
    tensor1d: jest.fn(),
    tensor2d: jest.fn(),
    tensor3d: jest.fn(),
    tensor4d: jest.fn(),
    ones: jest.fn(),
    zeros: jest.fn(),
    memory: jest.fn(() => ({
        numBytes: 0,
        numTensors: 0,
        numDataBuffers: 0,
        unreliable: false
    })),
    dispose: jest.fn(),
    tidy: jest.fn((fn: () => any) => fn()),
    loadLayersModel: jest.fn(),
    sequential: jest.fn(),
    layers: {
        dense: jest.fn(),
        activation: jest.fn()
    },
    train: {
        adam: jest.fn()
    },
    losses: {
        meanSquaredError: jest.fn()
    }
}));

// Mock Three.js
jest.mock('three', () => ({
    WebGLRenderer: jest.fn().mockImplementation(() => ({
        setSize: jest.fn(),
        render: jest.fn(),
        dispose: jest.fn(),
        domElement: document.createElement('canvas')
    })),
    Scene: jest.fn(),
    PerspectiveCamera: jest.fn(),
    Vector3: jest.fn(),
    Vector2: jest.fn(),
    Euler: jest.fn(),
    Quaternion: jest.fn(),
    Matrix4: jest.fn(),
    Box3: jest.fn(),
    Sphere: jest.fn(),
    Color: jest.fn(),
    Object3D: jest.fn(),
    Group: jest.fn(),
    Mesh: jest.fn(),
    Line: jest.fn(),
    Points: jest.fn(),
    BufferGeometry: jest.fn(),
    Material: jest.fn(),
    LineBasicMaterial: jest.fn(),
    MeshBasicMaterial: jest.fn(),
    PointsMaterial: jest.fn(),
    TextureLoader: jest.fn(),
    LoadingManager: jest.fn(),
    Clock: jest.fn(() => ({
        getDelta: () => 0.016,
        getElapsedTime: () => 0
    }))
}));

// Suppress console errors during tests
const originalError = console.error;
beforeAll(() => {
    console.error = (...args: any[]) => {
        if (
            /Warning: ReactDOM.render is no longer supported in React 18/.test(args[0]) ||
            /Warning: useLayoutEffect does nothing on the server/.test(args[0])
        ) {
            return;
        }
        originalError.call(console, ...args);
    };
});

afterAll(() => {
    console.error = originalError;
    global.requestAnimationFrame = originalRAF;
});
