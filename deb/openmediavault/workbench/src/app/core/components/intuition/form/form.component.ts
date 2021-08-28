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
import { AfterViewInit, Component, Input, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup, ValidatorFn, Validators } from '@angular/forms';
import * as _ from 'lodash';

import {
  flattenFormFieldConfig,
  setupConfObjUuidFields
} from '~/app/core/components/intuition/functions.helper';
import {
  FormFieldConfig,
  FormFieldConstraintValidator
} from '~/app/core/components/intuition/models/form-field-config.type';
import { format } from '~/app/functions.helper';
import { CustomValidators } from '~/app/shared/forms/custom-validators';
import { ConstraintService } from '~/app/shared/services/constraint.service';

let nextUniqueId = 0;

@Component({
  selector: 'omv-intuition-form',
  templateUrl: './form.component.html',
  styleUrls: ['./form.component.scss']
})
export class FormComponent implements AfterViewInit, OnInit {
  @Input()
  id: string;

  @Input()
  config: FormFieldConfig[];

  @Input()
  context = {};

  public formGroup: FormGroup;

  constructor(private formBuilder: FormBuilder) {}

  ngOnInit(): void {
    this.sanitizeConfig();
    this.createForm();
  }

  ngAfterViewInit(): void {
    // All form fields that are involved in a 'visible' or 'hidden' modifier
    // must be updated. This will trigger the evaluation of the constraint
    // which finally sets the correct (configured) state of the form field
    // after form initialization.
    const allFields: Array<FormFieldConfig> = flattenFormFieldConfig(this.config);
    const fieldsToUpdate: Array<string> = [];
    _.forEach(allFields, (field: FormFieldConfig) => {
      _.forEach(field?.modifiers, (modifier) => {
        if (['visible', 'hidden'].includes(modifier.type)) {
          // Determine the fields involved in the constraint.
          const fields = ConstraintService.getFields(modifier.constraint);
          fieldsToUpdate.push(...fields);
        }
      });
    });
    _.forEach(_.uniq(fieldsToUpdate), (path) => {
      const control = this.formGroup.get(path);
      control.updateValueAndValidity({ onlySelf: true, emitEvent: true });
    });
  }

  protected sanitizeConfig() {
    // Create unique form identifier.
    this.id = _.defaultTo(this.id, `omv-intuition-form-${++nextUniqueId}`);
    // Populate the data model identifier field.
    setupConfObjUuidFields(this.config);
  }

  private createForm() {
    const controlsConfig = {};
    const allFields: Array<FormFieldConfig> = flattenFormFieldConfig(this.config);
    _.forEach(allFields, (field: FormFieldConfig) => {
      const validators: Array<ValidatorFn> = [];
      // Build the validator configuration.
      if (_.isArray(field.modifiers)) {
        _.forEach(field.modifiers, (modifier) => {
          validators.push(
            CustomValidators.modifyIf(
              modifier.type,
              modifier.typeConfig,
              _.defaultTo(modifier.opposite, true),
              modifier.constraint,
              this.context
            )
          );
        });
      }
      if (_.isPlainObject(field.validators)) {
        if (_.isBoolean(field.validators.required) && field.validators.required) {
          validators.push(Validators.required);
        }
        if (_.isNumber(field.validators.minLength) && field.validators.minLength > 0) {
          validators.push(Validators.minLength(field.validators.minLength));
        }
        if (_.isNumber(field.validators.maxLength && field.validators.maxLength > 0)) {
          validators.push(Validators.maxLength(field.validators.maxLength));
        }
        if (_.isNumber(field.validators.min)) {
          validators.push(Validators.min(field.validators.min));
        }
        if (_.isNumber(field.validators.max)) {
          validators.push(Validators.max(field.validators.max));
        }
        if (_.isPlainObject(field.validators.pattern)) {
          validators.push(
            CustomValidators.pattern(
              field.validators.pattern.pattern,
              field.validators.pattern.errorData
            )
          );
        }
        if (_.isBoolean(field.validators.email) && field.validators.email) {
          validators.push(Validators.email);
        }
        if (_.isPlainObject(field.validators.requiredIf)) {
          validators.push(CustomValidators.requiredIf(field.validators.requiredIf));
        }
        if (_.isArray(field.validators.custom)) {
          _.forEach(field.validators.custom, (custom: FormFieldConstraintValidator) => {
            validators.push(
              CustomValidators.constraint(
                custom.constraint,
                this.context,
                custom.errorCode,
                custom.errorData
              )
            );
          });
        }
        if (_.isString(field.validators.patternType)) {
          validators.push(CustomValidators.patternType(field.validators.patternType));
        }
      }
      let value = _.defaultTo(field.value, null);
      if (_.isString(value)) {
        // Evaluate filters.
        value = format(value, {});
      }
      // Create the form control.
      controlsConfig[field.name] = new FormControl(
        { value, disabled: _.defaultTo(field.disabled, false) },
        { validators, updateOn: field.updateOn }
      );
    });
    this.formGroup = this.formBuilder.group(controlsConfig);
  }
}
