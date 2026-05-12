import { flattenFormFieldConfig } from '~/app/core/components/intuition/functions.helper';
import { FormFieldConfig } from '~/app/core/components/intuition/models/form-field-config.type';

describe('functions', () => {
  it('should flatten form field config (1)', () => {
    const fields: Array<FormFieldConfig> = [
      {
        type: 'textInput',
        name: 'a'
      },
      {
        type: 'container',
        name: 'b',
        fields: [
          {
            type: 'numberInput',
            name: 'c'
          },
          {
            type: 'passwordInput',
            name: 'd'
          }
        ]
      }
    ];
    // @ts-ignore
    expect(flattenFormFieldConfig(fields)).toEqual([
      {
        type: 'textInput',
        name: 'a'
      },
      {
        type: 'numberInput',
        name: 'c'
      },
      {
        type: 'passwordInput',
        name: 'd'
      }
    ]);
  });
});
