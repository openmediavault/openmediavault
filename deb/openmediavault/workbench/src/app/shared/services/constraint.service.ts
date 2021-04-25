/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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

import { format, isUUIDv4 } from '~/app/functions.helper';
import {
  Constraint,
  ConstraintProperty,
  ConstraintRecord,
  ConstraintValue
} from '~/app/shared/models/constraint.type';

export class ConstraintService {
  /**
   * Returns the fields used in the constraint.
   *
   * @param constraint The constraint to process.
   * @return Returns a list of field names.
   */
  static getFields(constraint: Constraint): Array<string> {
    const innerGetFields = (node: any): Array<string> => {
      let result = [];
      if (_.isPlainObject(node)) {
        if (_.has(node, 'prop')) {
          node = node as ConstraintProperty;
          result.push(node.prop);
        } else if (_.has(node, 'operator')) {
          node = node as Constraint;
          switch (node.operator) {
            case '&&':
            case 'and':
            case '||':
            case 'or':
            case 'if':
            case '===':
            case 'eq':
            case '!==':
            case 'ne':
            case '==':
            case '!=':
            case '<':
            case 'lt':
            case '<=':
            case 'le':
            case '>':
            case 'gt':
            case '>=':
            case 'ge':
            case 'startsWith':
            case 'endsWith':
            case 'regex':
              result = _.concat(innerGetFields(node.arg0), innerGetFields(node.arg1));
              break;
            case '!':
            case 'not':
            case 'empty':
            case 'z':
            case 'notEmpty':
            case 'n':
            case 'in':
            case 'length':
            case 'UUIDv4':
            case 'truthy':
            case 'falsy':
            case 'has':
              result = innerGetFields(node.arg0);
              break;
          }
        }
      }
      return result;
    };
    return _.uniq(innerGetFields(constraint));
  }

  /**
   * Test the constraint.
   *
   * @param constraint The constraint to process.
   * @param object The object containing the data to test.
   * @return Returns true if all constrains are fulfilled, otherwise false.
   */
  static test(constraint: Constraint, object: ConstraintRecord): any {
    const innerTest = (data: ConstraintRecord, node: any) => {
      let result;
      if (_.isPlainObject(node)) {
        if (_.has(node, 'prop')) {
          node = node as ConstraintProperty;
          result = _.get(data, node.prop);
        } else if (_.has(node, 'value')) {
          node = node as ConstraintValue;
          result = format(node.value, data);
        } else if (_.has(node, 'operator')) {
          let arg0;
          let arg1;
          node = node as Constraint;
          switch (node.operator) {
            case '&&':
            case 'and':
              result = innerTest(data, node.arg0) && innerTest(data, node.arg1);
              break;
            case '||':
            case 'or':
              result = innerTest(data, node.arg0) || innerTest(data, node.arg1);
              break;
            case '!':
            case 'not':
              result = !innerTest(data, node.arg0);
              break;
            case 'if':
              result = !innerTest(data, node.arg0) ? true : innerTest(data, node.arg1);
              break;
            case '==':
            case '===':
            case 'eq':
              result = innerTest(data, node.arg0) === innerTest(data, node.arg1);
              break;
            case '!=':
            case '!==':
            case 'ne':
              result = innerTest(data, node.arg0) !== innerTest(data, node.arg1);
              break;
            case '<':
            case 'lt':
              result = innerTest(data, node.arg0) < innerTest(data, node.arg1);
              break;
            case '<=':
            case 'le':
              result = innerTest(data, node.arg0) <= innerTest(data, node.arg1);
              break;
            case '>':
            case 'gt':
              result = innerTest(data, node.arg0) > innerTest(data, node.arg1);
              break;
            case '>=':
            case 'ge':
              result = innerTest(data, node.arg0) >= innerTest(data, node.arg1);
              break;
            case 'empty':
            case 'z':
              result = _.isEmpty(innerTest(data, node.arg0));
              break;
            case 'notEmpty':
            case 'n':
              result = !_.isEmpty(innerTest(data, node.arg0));
              break;
            case 'in':
              arg0 = innerTest(data, node.arg0);
              arg1 = innerTest(data, node.arg1);
              if (_.isString(arg1)) {
                result = (arg1 as string).search(arg0) !== -1;
              } else {
                // Array
                result = -1 !== _.indexOf(arg1, arg0);
              }
              break;
            case 'startsWith':
              result = _.startsWith(innerTest(data, node.arg0), innerTest(data, node.arg1));
              break;
            case 'endsWith':
              result = _.endsWith(innerTest(data, node.arg0), innerTest(data, node.arg1));
              break;
            case 'regex':
              arg0 = innerTest(data, node.arg0);
              arg1 = innerTest(data, node.arg1);
              result = RegExp(arg1 as string).exec(arg0) !== null;
              break;
            case 'length':
              arg0 = innerTest(data, node.arg0);
              result = _.get(arg0, 'length', 0);
              break;
            case 'UUIDv4':
              result = isUUIDv4(innerTest(data, node.arg0));
              break;
            case 'truthy':
              result = _.includes([1, 'true', true, 'yes', 'y'], innerTest(data, node.arg0));
              break;
            case 'falsy':
              // https://developer.mozilla.org/en-US/docs/Glossary/Falsy
              result = _.includes(
                [0, 'false', false, 'no', 'n', undefined, null, NaN, ''],
                innerTest(data, node.arg0)
              );
              break;
            case 'has':
              // arg0 is pointing to a field. If the field does not exists,
              // then 'undefined' is returned.
              arg0 = innerTest(data, node.arg0);
              result = !_.isUndefined(arg0);
              break;
          }
        }
      } else {
        if (_.isString(node)) {
          // Evaluate filters.
          result = format(node, data);
        } else {
          result = node;
        }
      }
      return result;
    };
    return innerTest(object, constraint);
  }

  /**
   * Filter all objects that meet the condition.
   *
   * @param objects The objects to be filtered.
   * @param constraint The constraint to process.
   * @return Return a list ob objects that meet the specified condition.
   */
  static filter(objects: Array<ConstraintRecord>, constraint: Constraint): Array<ConstraintRecord> {
    return _.filter(objects, (object) => ConstraintService.test(constraint, object));
  }
}
