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

import { format, isFormatable, isUUID } from '~/app/functions.helper';
import {
  Constraint,
  ConstraintProperty,
  ConstraintRecord,
  ConstraintValue
} from '~/app/shared/models/constraint.type';

export class ConstraintService {
  /**
   * Determine the properties involved in the given constraint.
   *
   * @param constraint The constraint to process.
   * @return Returns a list of properties.
   */
  static getProps(constraint: Constraint): Array<string> {
    const innerGetProps = (node: any): Array<string> => {
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
            case '<>':
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
              result = _.concat(innerGetProps(node.arg0), innerGetProps(node.arg1));
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
              result = innerGetProps(node.arg0);
              break;
          }
        }
      }
      return result;
    };
    return _.uniq(innerGetProps(constraint));
  }

  /**
   * Evaluate the constraint.
   *
   * @param constraint The constraint to process.
   * @param object The object containing the data to test.
   * @return Returns `true` or 'any' if the constraint is fulfilled,
   *   otherwise `false`.
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
          result = isFormatable(node.value) ? format(node.value, data) : node.value;
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
            case '<>':
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
              result = _.includes(arg1, arg0);
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
            case 'UUID':
              result = isUUID(innerTest(data, node.arg0));
              break;
            case 'truthy':
              result = _.includes(
                [1, 'TRUE', 'True', 'true', true, 'YES', 'Yes', 'yes', 'Y', 'y'],
                innerTest(data, node.arg0)
              );
              break;
            case 'falsy':
              // https://developer.mozilla.org/en-US/docs/Glossary/Falsy
              result = _.includes(
                [
                  0,
                  'FALSE',
                  'False',
                  'false',
                  false,
                  'NO',
                  'No',
                  'no',
                  'N',
                  'n',
                  undefined,
                  null,
                  NaN,
                  ''
                ],
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
   * Iterates over elements of collection, returning an array of all elements
   * the constraint returns truthy for.
   *
   * @param objects The collection to iterate over.
   * @param constraint The constraint to process.
   * @return Returns the new filtered array.
   */
  static filter(objects: Array<ConstraintRecord>, constraint: Constraint): Array<ConstraintRecord> {
    return _.filter(objects, (object) => ConstraintService.test(constraint, object));
  }
}
