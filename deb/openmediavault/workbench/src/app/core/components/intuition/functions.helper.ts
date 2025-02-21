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

import { FormFieldConfig } from '~/app/core/components/intuition/models/form-field-config.type';
import { PageContext } from '~/app/core/models/page-context.type';
import { formatDeep, isFormatable } from '~/app/functions.helper';

/**
 * Flatten the configuration the get all fields, including the ones used
 * in 'containers'.
 */
export const flattenFormFieldConfig = (fields: Array<FormFieldConfig>): Array<FormFieldConfig> =>
  _.flatMap(
    _.filter(
      fields,
      (field: FormFieldConfig) => !_.isUndefined(field.name) || _.isArray(field.fields)
    ),
    (field: FormFieldConfig) => {
      if (_.isArray(field.fields)) {
        return flattenFormFieldConfig(field.fields);
      } else {
        return field;
      }
    }
  );

/**
 * Helper function to format the specified tokenized form field properties.
 * @param fields The list of form fields.
 * @param pageContext The page context.
 * @param props The list of tokenized properties to format.
 * @param convertFn The function that is used to convert the formatted string.
 *   E.g. this can be used to convert `string` to `boolean`.
 */
export const formatFormFieldConfig = (
  fields: Array<FormFieldConfig>,
  pageContext: PageContext,
  props: Array<string>,
  convertFn?: any
): void => {
  _.forEach(fields, (field: FormFieldConfig) => {
    _.forEach(props, (prop) => {
      const value = _.get(field, prop);
      if (isFormatable(value)) {
        let newValue = formatDeep(value, pageContext);
        if (_.isFunction(convertFn)) {
          newValue = convertFn(newValue);
        }
        _.set(field, prop, newValue);
      }
    });
  });
};

/**
 * Helper function to set up 'confObjUuid' form fields.
 */
export const setupConfObjUuidFields = (fields: Array<FormFieldConfig>) => {
  const filteredFields = _.filter(fields, { type: 'confObjUuid' });
  if (filteredFields.length > 1) {
    throw new Error("Only one 'confObjUuid' field per form is allowed.");
  }
  if (filteredFields.length === 1) {
    // Set the UUID that is used to tell the backend that the
    // configuration object is new. The UUID will be replaced
    // by another one automatically.
    _.defaults(filteredFields[0], {
      name: 'uuid',
      value: '{{ _routeParams.uuid | default(newconfobjuuid) }}'
    });
  }
};
