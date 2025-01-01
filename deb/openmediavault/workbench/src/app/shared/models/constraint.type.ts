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
/**
 * .------------------------------------------------------------------.
 * | operator      | arg0               | arg1                        |
 * |---------------|--------------------|-----------------------------|
 * | &&  || and    | constraint         | constraint                  |
 * | ||  || or     | constraint         | constraint                  |
 * | !   || not    | constraint         |                             |
 * | if            | constraint         | constraint                  |
 * | === || eq     | constraint || prop | constraint || prop || value |
 * | !== || ne     | constraint || prop | constraint || prop || value |
 * | ==            | constraint || prop | constraint || prop || value |
 * | !=            | constraint || prop | constraint || prop || value |
 * | <   || lt     | constraint || prop | constraint || prop || value |
 * | <=  || le     | constraint || prop | constraint || prop || value |
 * | >   || gt     | constraint || prop | constraint || prop || value |
 * | >=  || ge     | constraint || prop | constraint || prop || value |
 * | empty    || z | constraint || prop |                             |
 * | notEmpty || n | constraint || prop |                             |
 * | in            | constraint || prop | prop || array || value      |
 * | startsWith    | constraint || prop | value                       |
 * | endsWith      | constraint || prop | value                       |
 * | regex         | constraint || prop | value                       |
 * | length        | prop || value      |                             |
 * | UUID          | prop || value      |                             |
 * | truthy        | prop || value      |                             |
 * | falsy         | prop || value      |                             |
 * | has           | prop               |                             |
 * '------------------------------------------------------------------'
 *
 * The 'if' operator evaluates the 'arg1' constraint only if 'arg0'
 * succeeds: If 'arg0' is falsy, the whole constraint will succeed.
 *
 * The 'length' operator will return the length of an array or string.
 * Make sure to use this operator only for those types.
 *
 * The 'has' operator checks whether the specified prop exists.
 *
 * Example 1:
 * {
 *   operator: 'eq',
 *   arg0: {
 *     prop: 'password'
 *   },
 *   arg1: {
 *     prop: 'passwordconf'
 *   }
 * }
 * Example 2:
 * {
 *   operator: 'ne',
 *   arg0: {
 *     prop: 'password'
 *   },
 *   arg1: '123456789'
 * }
 * Example 3:
 * {
 *   operator: 'in',
 *   arg0: {
 *     prop: 'language'
 *   },
 *   arg1: ['de', 'en', 'fr']
 * }
 * Example 4:
 * {
 *   operator: 'if',
 *   arg0: { operator: 'has', arg0: { prop: '_used' } },
 *   arg1: { operator: 'falsy', arg0: { prop: '_used' } }
 * }
 */
export type Constraint = {
  operator:
    | '&&'
    | 'and'
    | '||'
    | 'or'
    | '!'
    | 'not'
    | 'if'
    | '==='
    | 'eq'
    | '!=='
    | 'ne'
    | '<>'
    | '=='
    | '!='
    | '<'
    | 'lt'
    | '<='
    | 'le'
    | '>'
    | 'gt'
    | '>='
    | 'ge'
    | 'empty'
    | 'z'
    | 'notEmpty'
    | 'n'
    | 'in'
    | 'startsWith'
    | 'endsWith'
    | 'regex'
    | 'length'
    | 'UUID'
    | 'truthy'
    | 'falsy'
    | 'has';
  arg0: Constraint | ConstraintProperty | ConstraintValue;
  arg1?:
    | boolean
    | number
    | string
    | Array<number | string>
    | Constraint
    | ConstraintProperty
    | ConstraintValue;
};

export type ConstraintProperty = {
  // The path of the property, e.g. 'x' or 'x.y.z'.
  prop: string;
};

export type ConstraintValue = {
  value: boolean | number | string | Array<number | string>;
};

export type ConstraintRecord = Record<any, any>;
