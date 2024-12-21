import '@testing-library/jest-dom';
import { expect, jest } from '@jest/globals';

declare global {
  const expect: typeof import('@jest/globals').expect;
  const jest: typeof import('@jest/globals').jest;

  namespace jest {
    interface Matchers<R> {
      toBeInTheDocument(): R;
      toHaveTextContent(text: string | RegExp): R;
      toHaveAttribute(attr: string, value?: string): R;
      toHaveValue(value: string | number | string[]): R;
      toHaveFocus(): R;
      toHaveAccessibleName(name?: string): R;
    }
  }
}
