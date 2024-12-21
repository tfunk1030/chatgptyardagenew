import '@testing-library/jest-dom';
import { TestingLibraryMatchers } from '@testing-library/jest-dom/matchers';

declare global {
    namespace jest {
        interface Matchers<R = void, T = {}> extends TestingLibraryMatchers<typeof expect.stringContaining, T> {}
    }
}

declare module '@testing-library/jest-dom' {
    export interface Matchers<R = void, T = {}> {
        toBeInTheDocument(): R;
        toHaveTextContent(text: string | RegExp): R;
        toHaveAttribute(attr: string, value?: string): R;
        toHaveValue(value: string | number | string[]): R;
        toHaveFocus(): R;
        toHaveAccessibleName(name?: string): R;
    }
}

export {};
