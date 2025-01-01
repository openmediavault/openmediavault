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
import * as _ from 'lodash';
import { iif, interval, Observable, range, throwError, zip } from 'rxjs';
import { filter, mergeMap, retryWhen, take } from 'rxjs/operators';

/**
 * Returns an Observable that mirrors the source Observable with the
 * exception of an error. If the source Observable calls error, this
 * method will resubscribe to the source Observable for a maximum of
 * count resubscriptions (given as a number parameter) rather than
 * propagating the error call.
 *
 * @param count Number of retry attempts before failing. Defaults to 0.
 * @param period The delay duration in milliseconds until which the
 *   emission of the source item is delayed. With every iteration
 *   of `count` the delay will be increased by N * delay.
 *   Defaults to 1000 milliseconds.
 */
// eslint-disable-next-line prefer-arrow/prefer-arrow-functions
export function retryDelayed<T>(count?: number, period: number = 1000) {
  count = _.defaultTo(count, 0);
  return (source: Observable<T>): Observable<T> =>
    source.pipe(
      retryWhen((errors: Observable<any>) =>
        zip(range(1, count + 1), errors).pipe(
          mergeMap(([i, error]) =>
            iif(
              () => {
                const result = i < count + 1;
                if (result) {
                  error?.preventDefault?.();
                }
                return result;
              },
              interval(i * period),
              throwError(error)
            )
          )
        )
      )
    );
}

/**
 * Emits the first value emitted by the source Observable as soon
 * as the condition becomes true.
 *
 * @param predicate A function to test each item emitted from the
 *   source Observable.
 */
// eslint-disable-next-line prefer-arrow/prefer-arrow-functions
export function takeWhen<T>(predicate: (value: T, index: number) => boolean) {
  return (source: Observable<T>): Observable<T> => source.pipe(filter(predicate), take(1));
}
