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
 * | UUIDv4        | prop || value      |                             |
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
    | 'UUIDv4'
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
  prop: string;
};

export type ConstraintValue = {
  value: boolean | number | string | Array<number | string>;
};

export type ConstraintRecord = Record<any, any>;
