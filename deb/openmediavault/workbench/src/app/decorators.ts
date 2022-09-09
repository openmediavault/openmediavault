/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2022 Volker Theile
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
import * as _ from 'lodash';

/**
 * Decorator to automatically unsubscribe subscriptions.
 */
// eslint-disable-next-line @typescript-eslint/naming-convention,prefer-arrow/prefer-arrow-functions
export function Unsubscribe() {
  return (constructor) => {
    const originalFn = constructor.prototype.ngOnDestroy;
    // eslint-disable-next-line prefer-arrow/prefer-arrow-functions
    constructor.prototype.ngOnDestroy = function() {
      _.forEach(this, (value, key) => {
        const property = this[key];
        if (_.isFunction(property.unsubscribe)) {
          property.unsubscribe();
        }
      });
      originalFn?.apply();
    };
  };
}

/**
 * Decorator that creates a debounced function that delays invoking func
 * until after wait milliseconds have elapsed since the last time the
 * debounced function was invoked.
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
