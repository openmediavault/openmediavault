import * as _ from 'lodash';

/**
 * Decorator to automatically unsubscribe subscriptions.
 */
// eslint-disable-next-line @typescript-eslint/naming-convention
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
