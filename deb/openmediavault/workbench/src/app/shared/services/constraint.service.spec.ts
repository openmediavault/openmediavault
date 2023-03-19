import { TestBed } from '@angular/core/testing';

import { ConstraintRecord } from '~/app/shared/models/constraint.type';
import { ConstraintService } from '~/app/shared/services/constraint.service';

describe('ConstraintService', () => {
  let object: ConstraintRecord;
  let objects: Array<ConstraintRecord>;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    object = {
      password0: 'foo',
      password1: '123456789',
      passwordconf: 'test',
      language: 'de',
      locales: ['en', 'fr', 'it'],
      uuid: '1fb08050-1aa9-11ea-a4f8-8786b93c428c',
      disabled: true,
      visible: false,
      emptyDict: [],
      emptyString: ''
    };
    objects = [
      {
        a: '',
        b: '12abc',
        x: 'foo',
        y: 12,
        z: false
      }
    ];
  });

  it('should test (1)', () => {
    const result = ConstraintService.test(
      {
        operator: 'and',
        arg0: { operator: 'eq', arg0: { prop: 'language' }, arg1: 'de' },
        arg1: { operator: 'ne', arg0: { prop: 'password1' }, arg1: 'bar' }
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (2)', () => {
    const result = ConstraintService.test(
      {
        operator: 'or',
        arg0: { operator: 'ne', arg0: { prop: 'language' }, arg1: 'de' },
        arg1: { operator: 'eq', arg0: { prop: 'password1' }, arg1: 'bar' }
      },
      object
    );
    expect(result).toBeFalsy();
  });

  it('should test (3)', () => {
    const result = ConstraintService.test(
      {
        operator: '!',
        arg0: { operator: 'eq', arg0: { prop: 'language' }, arg1: 'de' }
      },
      object
    );
    expect(result).toBeFalsy();
  });

  it('should test (4)', () => {
    const result = ConstraintService.test(
      {
        operator: 'eq',
        arg0: { prop: 'language' },
        arg1: 'de'
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (5)', () => {
    const result = ConstraintService.test(
      {
        operator: 'ne',
        arg0: { prop: 'password0' },
        arg1: '123456789'
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (6)', () => {
    const result = ConstraintService.test(
      {
        operator: 'in',
        arg0: { prop: 'language' },
        arg1: ['de', 'en', 'fr']
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (7)', () => {
    const result = ConstraintService.test(
      {
        operator: 'in',
        arg0: { prop: 'language' },
        arg1: { prop: 'locales' }
      },
      object
    );
    expect(result).toBeFalsy();
  });

  it('should test (8)', () => {
    const result = ConstraintService.test(
      {
        operator: 'UUID',
        arg0: { prop: 'uuid' }
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (9)', () => {
    const result = ConstraintService.test(
      {
        operator: 'UUID',
        arg0: { prop: 'language' }
      },
      object
    );
    expect(result).toBeFalsy();
  });

  it('should test (10)', () => {
    const result = ConstraintService.test(
      {
        operator: 'truthy',
        arg0: { prop: 'disabled' }
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (11)', () => {
    const result = ConstraintService.test(
      {
        operator: 'truthy',
        arg0: { prop: 'visible' }
      },
      object
    );
    expect(result).toBeFalsy();
  });

  it('should test (12)', () => {
    const result = ConstraintService.test(
      {
        operator: 'falsy',
        arg0: { prop: 'disabled' }
      },
      object
    );
    expect(result).toBeFalsy();
  });

  it('should test (13)', () => {
    const result = ConstraintService.test(
      {
        operator: 'falsy',
        arg0: { prop: 'visible' }
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (14)', () => {
    const result = ConstraintService.test(
      {
        operator: 'notEmpty',
        arg0: { prop: 'locales' }
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (15)', () => {
    const result = ConstraintService.test(
      {
        operator: 'notEmpty',
        arg0: { prop: 'uuid' }
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (16)', () => {
    const result = ConstraintService.test(
      {
        operator: 'notEmpty',
        arg0: { prop: 'emptyString' }
      },
      object
    );
    expect(result).toBeFalsy();
  });

  it('should test (17)', () => {
    const result = ConstraintService.test(
      {
        operator: 'empty',
        arg0: { prop: 'locales' }
      },
      object
    );
    expect(result).toBeFalsy();
  });

  it('should test (18)', () => {
    const result = ConstraintService.test(
      {
        operator: 'empty',
        arg0: { prop: 'uuid' }
      },
      object
    );
    expect(result).toBeFalsy();
  });

  it('should test (19)', () => {
    const result = ConstraintService.test(
      {
        operator: 'empty',
        arg0: { prop: 'emptyString' }
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (20)', () => {
    const result = ConstraintService.test(
      {
        operator: 'eq',
        arg0: {
          operator: 'length',
          arg0: {
            prop: 'password0'
          }
        },
        arg1: 3
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (21)', () => {
    const result = ConstraintService.test(
      {
        operator: '>',
        arg0: {
          operator: 'length',
          arg0: {
            prop: 'password1'
          }
        },
        arg1: 3
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (22)', () => {
    const result = ConstraintService.test(
      {
        operator: '>=',
        arg0: {
          operator: 'length',
          arg0: {
            prop: 'locales'
          }
        },
        arg1: 4
      },
      object
    );
    expect(result).toBeFalsy();
  });

  it('should test (23)', () => {
    const result = ConstraintService.test(
      {
        operator: 'if',
        arg0: {
          operator: 'truthy',
          arg0: { prop: 'visible' }
        },
        arg1: {
          operator: '===',
          arg0: { prop: 'password1' },
          arg1: '123456789'
        }
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (24)', () => {
    const result = ConstraintService.test(
      {
        operator: 'if',
        arg0: {
          operator: 'truthy',
          arg0: { prop: 'disabled' }
        },
        arg1: {
          operator: '===',
          arg0: { prop: 'password1' },
          arg1: '123456789'
        }
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (25)', () => {
    const result = ConstraintService.test(
      {
        operator: 'truthy',
        arg0: { value: '{{ disabled | toboolean }}' }
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (26)', () => {
    const result = ConstraintService.test(
      {
        operator: 'falsy',
        arg0: { prop: 'foo' }
      },
      { bar: true }
    );
    expect(result).toBeTruthy();
  });

  it('should test (27)', () => {
    const result = ConstraintService.test(
      {
        operator: 'falsy',
        arg0: { prop: 'foo' }
      },
      { foo: undefined }
    );
    expect(result).toBeTruthy();
  });

  it('should test (28)', () => {
    const result = ConstraintService.test(
      {
        operator: 'falsy',
        arg0: { prop: 'foo' }
      },
      { foo: null }
    );
    expect(result).toBeTruthy();
  });

  it('should test (29)', () => {
    const result = ConstraintService.test(
      {
        operator: 'falsy',
        arg0: { prop: 'foo' }
      },
      { foo: NaN }
    );
    expect(result).toBeTruthy();
  });

  it('should test (30)', () => {
    const result = ConstraintService.test(
      {
        operator: 'falsy',
        arg0: { prop: 'foo' }
      },
      { foo: '' }
    );
    expect(result).toBeTruthy();
  });

  it('should test (31)', () => {
    const result = ConstraintService.test(
      {
        operator: 'in',
        arg0: { value: 'fr' },
        arg1: { prop: 'locales' }
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (32)', () => {
    const result = ConstraintService.test(
      {
        operator: 'in',
        arg0: { value: '*' },
        arg1: { prop: 'locales' }
      },
      object
    );
    expect(result).toBeFalsy();
  });

  it('should test (33)', () => {
    const result = ConstraintService.test(
      {
        operator: 'in',
        arg0: { prop: 'locales' },
        arg1: { prop: 'language' }
      },
      object
    );
    expect(result).toBeFalsy();
  });

  it('should test (34)', () => {
    const result = ConstraintService.test(
      {
        operator: 'in',
        arg0: { value: 'ccc' },
        arg1: { value: 'aaa bbb ccc' }
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should test (35)', () => {
    const result = ConstraintService.test(
      {
        operator: 'in',
        arg0: { value: 2 },
        arg1: [1, 2, 3, 4]
      },
      object
    );
    expect(result).toBeTruthy();
  });

  it('should get properties (1)', () => {
    const result = ConstraintService.getProps({
      operator: 'and',
      arg0: { operator: 'eq', arg0: { prop: 'field1' }, arg1: 'foo' },
      arg1: {
        operator: 'or',
        arg0: { operator: 'ne', arg0: { prop: 'field2' }, arg1: 'xyz' },
        arg1: { operator: 'eq', arg0: { prop: 'field3' }, arg1: 'bar' }
      }
    });
    expect(result).toEqual(['field1', 'field2', 'field3']);
  });

  it('should get properties (2)', () => {
    const result = ConstraintService.getProps({
      operator: 'ne',
      arg0: { prop: 'password0' },
      arg1: { prop: 'passwordconf' }
    });
    expect(result).toEqual(['password0', 'passwordconf']);
  });

  it('should get properties (3)', () => {
    const result = ConstraintService.getProps({
      operator: 'not',
      arg0: {
        operator: 'in',
        arg0: { prop: 'password0' },
        arg1: ['123456789', 'password']
      }
    });
    expect(result).toEqual(['password0']);
  });

  it('should get properties (4)', () => {
    const result = ConstraintService.getProps({
      operator: 'if',
      arg0: {
        operator: 'truthy',
        arg0: { prop: 'disabled' }
      },
      arg1: {
        operator: '===',
        arg0: { prop: 'password1' },
        arg1: '123456789'
      }
    });
    expect(result).toEqual(['disabled', 'password1']);
  });

  it('should get properties (5)', () => {
    const result = ConstraintService.getProps({
      operator: '>=',
      arg0: {
        operator: 'length',
        arg0: {
          prop: 'locales'
        }
      },
      arg1: 4
    });
    expect(result).toEqual(['locales']);
  });

  it('should filter (1)', () => {
    const result = ConstraintService.filter([], {
      operator: '===',
      arg0: { prop: 'x' },
      arg1: 'foo'
    });
    expect(result.length).toBe(0);
  });

  it('should filter (2)', () => {
    const result = ConstraintService.filter(objects, {
      operator: '&&',
      arg0: {
        operator: '===',
        arg0: { prop: 'x' },
        arg1: 'foo'
      },
      arg1: {
        operator: '===',
        arg0: { prop: 'z' },
        arg1: false
      }
    });
    expect(result.length).toBe(1);
  });

  it('should filter (3)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'and',
      arg0: {
        operator: '===',
        arg0: { prop: 'x' },
        arg1: 'foo'
      },
      arg1: {
        operator: '===',
        arg0: { prop: 'z' },
        arg1: true
      }
    });
    expect(result.length).toBe(0);
  });

  it('should filter (4)', () => {
    const result = ConstraintService.filter(objects, {
      operator: '||',
      arg0: {
        operator: '===',
        arg0: { prop: 'x' },
        arg1: 'foo'
      },
      arg1: {
        operator: '===',
        arg0: { prop: 'z' },
        arg1: true
      }
    });
    expect(result.length).toBe(1);
  });

  it('should filter (5)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'not',
      arg0: {
        operator: '===',
        arg0: { prop: 'x' },
        arg1: 'foo'
      }
    });
    expect(result.length).toBe(0);
  });

  it('should filter (6)', () => {
    const result = ConstraintService.filter(objects, {
      operator: '===',
      arg0: { prop: 'x' },
      arg1: 'foo'
    });
    expect(result.length).toBe(1);
  });

  it('should filter (7)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'eq',
      arg0: { prop: 'x' },
      arg1: 'bar'
    });
    expect(result.length).toBe(0);
  });

  it('should filter (8)', () => {
    const result = ConstraintService.filter(objects, {
      operator: '!==',
      arg0: { prop: 'x' },
      arg1: 'bar'
    });
    expect(result.length).toBe(1);
  });

  it('should filter (9)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'ne',
      arg0: { prop: 'x' },
      arg1: 'foo'
    });
    expect(result.length).toBe(0);
  });

  it('should filter (10)', () => {
    const result = ConstraintService.filter(objects, {
      operator: '==',
      arg0: { prop: 'y' },
      arg1: '12'
    });
    expect(result.length).toBe(0);
  });

  it('should filter (11)', () => {
    const result = ConstraintService.filter(objects, {
      operator: '==',
      arg0: { prop: 'y' },
      arg1: '2'
    });
    expect(result.length).toBe(0);
  });

  it('should filter (12)', () => {
    const result = ConstraintService.filter(objects, {
      operator: '!=',
      arg0: { prop: 'y' },
      arg1: '2'
    });
    expect(result.length).toBe(1);
  });

  it('should filter (13)', () => {
    const result = ConstraintService.filter(objects, {
      operator: '!=',
      arg0: { prop: 'y' },
      arg1: '12'
    });
    expect(result.length).toBe(1);
  });

  it('should filter (14)', () => {
    const result = ConstraintService.filter(objects, {
      operator: '<',
      arg0: { prop: 'y' },
      arg1: 13
    });
    expect(result.length).toBe(1);
  });

  it('should filter (15)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'lt',
      arg0: { prop: 'y' },
      arg1: 11
    });
    expect(result.length).toBe(0);
  });

  it('should filter (16)', () => {
    const result = ConstraintService.filter(objects, {
      operator: '<=',
      arg0: { prop: 'y' },
      arg1: 12
    });
    expect(result.length).toBe(1);
  });

  it('should filter (17)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'le',
      arg0: { prop: 'y' },
      arg1: 11
    });
    expect(result.length).toBe(0);
  });

  it('should filter (18)', () => {
    const result = ConstraintService.filter(objects, {
      operator: '>',
      arg0: { prop: 'y' },
      arg1: 11
    });
    expect(result.length).toBe(1);
  });

  it('should filter (19', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'gt',
      arg0: { prop: 'y' },
      arg1: 12
    });
    expect(result.length).toBe(0);
  });

  it('should filter (20)', () => {
    const result = ConstraintService.filter(objects, {
      operator: '>=',
      arg0: { prop: 'y' },
      arg1: 12
    });
    expect(result.length).toBe(1);
  });

  it('should filter (21)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'ge',
      arg0: { prop: 'y' },
      arg1: 13
    });
    expect(result.length).toBe(0);
  });

  it('should filter (22)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'z',
      arg0: { prop: 'a' }
    });
    expect(result.length).toBe(1);
  });

  it('should filter (23)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'z',
      arg0: { prop: 'x' }
    });
    expect(result.length).toBe(0);
  });

  it('should filter (24)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'n',
      arg0: { prop: 'x' }
    });
    expect(result.length).toBe(1);
  });

  it('should filter (25)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'n',
      arg0: { prop: 'a' }
    });
    expect(result.length).toBe(0);
  });

  it('should filter (26)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'in',
      arg0: { prop: 'x' },
      arg1: ['foo', 'bar']
    });
    expect(result.length).toBe(1);
  });

  it('should filter (27)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'in',
      arg0: { prop: 'x' },
      arg1: ['1', '2']
    });
    expect(result.length).toBe(0);
  });

  it('should filter (28)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'in',
      arg0: { prop: 'x' },
      arg1: 'aaa foo bbb'
    });
    expect(result.length).toBe(1);
  });

  it('should filter (29)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'in',
      arg0: { prop: 'x' },
      arg1: 'dasda sd sdc'
    });
    expect(result.length).toBe(0);
  });

  it('should filter (30)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'startsWith',
      arg0: { prop: 'x' },
      arg1: 'fo'
    });
    expect(result.length).toBe(1);
  });

  it('should filter (31)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'startsWith',
      arg0: { prop: 'x' },
      arg1: 'aaa'
    });
    expect(result.length).toBe(0);
  });

  it('should filter (32)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'endsWith',
      arg0: { prop: 'x' },
      arg1: 'oo'
    });
    expect(result.length).toBe(1);
  });

  it('should filter (33)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'endsWith',
      arg0: { prop: 'x' },
      arg1: 'aaa'
    });
    expect(result.length).toBe(0);
  });

  it('should filter (34)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'regex',
      arg0: { prop: 'b' },
      arg1: '^12\\S+$'
    });
    expect(result.length).toBe(1);
  });

  it('should filter (35)', () => {
    const result = ConstraintService.filter(objects, {
      operator: 'regex',
      arg0: { prop: 'b' },
      arg1: '^abc'
    });
    expect(result.length).toBe(0);
  });
});
