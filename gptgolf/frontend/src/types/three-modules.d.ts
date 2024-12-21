declare module 'three/examples/jsm/controls/OrbitControls' {
    import { Camera, EventDispatcher, MOUSE, TOUCH } from 'three';

    export class OrbitControls extends EventDispatcher {
        constructor(camera: Camera, domElement?: HTMLElement);
        
        enabled: boolean;
        enableDamping: boolean;
        dampingFactor: number;
        enableZoom: boolean;
        enableRotate: boolean;
        enablePan: boolean;
        
        minDistance: number;
        maxDistance: number;
        minPolarAngle: number;
        maxPolarAngle: number;
        
        mouseButtons: {
            LEFT: MOUSE;
            MIDDLE: MOUSE;
            RIGHT: MOUSE;
        };
        
        touches: {
            ONE: TOUCH;
            TWO: TOUCH;
        };
        
        update(): boolean;
        dispose(): void;
    }
}

declare module 'three/examples/jsm/lines/Line2' {
    import { Material, Object3D, Vector3 } from 'three';
    import { LineGeometry } from 'three/examples/jsm/lines/LineGeometry';
    import { LineMaterial } from 'three/examples/jsm/lines/LineMaterial';

    export class Line2 extends Object3D {
        constructor(geometry?: LineGeometry, material?: LineMaterial);
        geometry: LineGeometry;
        material: LineMaterial;
        computeLineDistances(): this;
        raycast(raycaster: any, intersects: any[]): void;
        updateMorphTargets(): void;
    }
}

declare module 'three/examples/jsm/lines/LineGeometry' {
    import { InstancedBufferGeometry, Vector3 } from 'three';

    export class LineGeometry extends InstancedBufferGeometry {
        constructor();
        setPositions(array: number[]): this;
        setColors(array: number[]): this;
        fromLine(line: any): this;
        fromPoints(points: Vector3[]): this;
        dispose(): void;
    }
}

declare module 'three/examples/jsm/lines/LineMaterial' {
    import { Material, Vector2, Color, MaterialParameters } from 'three';

    export interface LineMaterialParameters extends MaterialParameters {
        color?: number | string | Color;
        linewidth?: number;
        resolution?: Vector2;
        dashed?: boolean;
        dashScale?: number;
        dashSize?: number;
        gapSize?: number;
        vertexColors?: boolean;
        worldUnits?: boolean;
    }

    export class LineMaterial extends Material {
        constructor(parameters?: LineMaterialParameters);
        
        color: Color;
        linewidth: number;
        resolution: Vector2;
        dashed: boolean;
        dashScale: number;
        dashSize: number;
        gapSize: number;
        vertexColors: boolean;
        worldUnits: boolean;
        
        isLineMaterial: boolean;
        
        copy(source: LineMaterial): this;
    }
}

// Add support for importing three.js examples
declare module 'three/examples/jsm/*' {
    const content: any;
    export default content;
}

// Add support for shader chunks
declare module 'three/src/renderers/shaders/*' {
    const content: string;
    export default content;
}
