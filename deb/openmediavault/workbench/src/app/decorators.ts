/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
import { coerceBooleanProperty } from '@angular/cdk/coercion';
import * as _ from 'lodash';

/**
 * A property decorator to automatically unsubscribe subscriptions.
 */
// eslint-disable-next-line @typescript-eslint/naming-convention,prefer-arrow/prefer-arrow-functions
export function Unsubscribe() {
  return (target: any, propertyKey: string) => {
    const originalFn = target.ngOnDestroy;
    // eslint-disable-next-line prefer-arrow/prefer-arrow-functions
    target.ngOnDestroy = function () {
      const property = this[propertyKey];
      property?.unsubscribe?.();
      originalFn?.apply?.(this, arguments);
    };
  };
}

/**
 * A method decorator that creates a debounced function that delays
 * invoking func until after wait milliseconds have elapsed since the
 * last time the debounced function was invoked.
 *
 * @param wait The number of milliseconds to delay.
 */
// eslint-disable-next-line @typescript-eslint/naming-convention,prefer-arrow/prefer-arrow-functions
export function Debounce(wait: number) {
  return (target: any, propertyKey: string, descriptor: PropertyDescriptor) => {
    const originalFn = descriptor.value;
    descriptor.value = _.debounce(originalFn, wait);
    return descriptor;
  };
}

/**
 * A method decorator that creates a throttled function that only
 * invokes func at most once per every wait milliseconds.
 *
 * @param wait The number of milliseconds to throttle invocations to.
 */
// eslint-disable-next-line @typescript-eslint/naming-convention,prefer-arrow/prefer-arrow-functions
export function Throttle(wait: number) {
  return (target: any, propertyKey: string, descriptor: PropertyDescriptor) => {
    const originalFn = descriptor.value;
    descriptor.value = _.throttle(originalFn, wait);
    return descriptor;
  };
}

/**
 * A property decorator to coercing a data-bound value (typically
 * a string) to a boolean.
 */
// eslint-disable-next-line @typescript-eslint/naming-convention,prefer-arrow/prefer-arrow-functions
export function CoerceBoolean() {
  return (target: any, propertyKey: string): any => {
    const _propertyKey = Symbol(propertyKey);
    target[_propertyKey] = target[propertyKey];
    Object.defineProperty(target, _propertyKey, {
      configurable: true,
      writable: true
    });
    return {
      get() {
        return this[_propertyKey];
      },
      set(value: any) {
        this[_propertyKey] = coerceBooleanProperty(value);
      }
    };
  };
}
