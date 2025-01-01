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
import { AfterViewInit, Component, Input, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn,
  Validators
} from '@angular/forms';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { Subscription } from 'rxjs';

import {
  flattenFormFieldConfig,
  setupConfObjUuidFields
} from '~/app/core/components/intuition/functions.helper';
import { FormFieldName } from '~/app/core/components/intuition/models/form.type';
import {
  FormFieldConfig,
  FormFieldConstraintValidator,
  FormFieldModifier
} from '~/app/core/components/intuition/models/form-field-config.type';
import { Unsubscribe } from '~/app/decorators';
import { format, formatDeep } from '~/app/functions.helper';
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

  @Unsubscribe()
  private subscriptions: Subscription = new Subscription();

  public formGroup: FormGroup;

  constructor(private formBuilder: FormBuilder) {}

  ngOnInit(): void {
    this.sanitizeConfig();
    this.createForm();
    this.initializeModifiers();
  }

  ngAfterViewInit(): void {
    // All form fields that are involved in a 'visible' or 'hidden' modifier
    // must be updated. This will trigger the evaluation of the constraint
    // which finally sets the correct (configured) state of the form field
    // after form initialization.
    const allFields: FormFieldConfig[] = flattenFormFieldConfig(this.config);
    const fieldNamesToUpdate: FormFieldName[] = [];
    _.forEach(allFields, (field: FormFieldConfig) => {
      _.forEach(
        _.filter(field?.modifiers, (modifier: FormFieldModifier) =>
          ['visible', 'hidden'].includes(modifier.type)
        ),
        (modifier: FormFieldModifier) => {
          // Determine the list of form fields that are involved in this
          // constraint.
          const fieldNames = ConstraintService.getProps(modifier.constraint);
          fieldNamesToUpdate.push(...fieldNames);
        }
      );
    });
    _.forEach(_.uniq(fieldNamesToUpdate), (name: FormFieldName) => {
      const control: AbstractControl = this.formGroup.get(name);
      control?.updateValueAndValidity({ onlySelf: true, emitEvent: true });
    });
    // Trigger the modifiers to apply and display the current state.
    // This is necessary because there are situations where a form
    // does not trigger any `valueChanges` event during the whole
    // initialization.
    this.applyModifiers();
  }

  protected sanitizeConfig(): void {
    // Create unique form identifier.
    this.id = _.defaultTo(this.id, `omv-intuition-form-${++nextUniqueId}`);
    // Sanitize the configuration of individual form fields.
    const allFields: Array<FormFieldConfig> = flattenFormFieldConfig(this.config);
    _.forEach(allFields, (field: FormFieldConfig) => {
      switch (field.type) {
        case 'binaryUnitInput':
          _.defaultsDeep(field, {
            defaultUnit: 'B',
            fractionDigits: 0,
            validators: {
              patternType: 'binaryUnit'
            }
          });
          break;
        case 'datatable':
          _.defaultsDeep(field, {
            columnMode: 'flex',
            hasHeader: true,
            hasFooter: true,
            selectionType: 'multi',
            limit: 25,
            columns: [],
            actions: [],
            sorters: [],
            sortType: 'single',
            valueType: 'object'
          });
          break;
        case 'fileInput':
          _.defaultsDeep(field, {
            autocapitalize: 'none',
            rows: 4,
            wrap: 'soft',
            trim: false
          });
          break;
        case 'folderBrowser':
          _.defaultsDeep(field, {
            autocapitalize: 'none',
            dirVisible: false
          });
          break;
        case 'select':
        case 'sharedFolderSelect':
        case 'sshCertSelect':
        case 'sslCertSelect':
          _.defaultsDeep(field, {
            valueField: 'value',
            textField: 'text',
            hasEmptyOption: false,
            emptyOptionText: gettext('None'),
            store: {
              data: []
            }
          });
          if (_.isArray(field.store.data) && _.isUndefined(field.store.fields)) {
            _.merge(field.store, {
              fields: _.uniq([field.valueField, field.textField])
            });
          }
          if (['sharedFolderSelect', 'sshCertSelect', 'sslCertSelect'].includes(field.type)) {
            _.defaultsDeep(field, {
              hasCreateButton: true
            });
          }
          break;
        case 'passwordInput':
          _.defaultsDeep(field, {
            autocapitalize: 'none'
          });
          break;
        case 'textInput':
          _.defaultsDeep(field, {
            autocapitalize: 'none',
            valueField: 'value',
            textField: 'text',
            suggestions: false,
            store: {
              data: []
            }
          });
          if (_.isArray(field.store.data) && _.isUndefined(field.store.fields)) {
            _.merge(field.store, {
              fields: _.uniq([field.valueField, field.textField])
            });
          }
          break;
        case 'textarea':
          _.defaultsDeep(field, {
            autocapitalize: 'none',
            rows: 4,
            wrap: 'soft'
          });
          break;
        case 'hint':
          _.defaultsDeep(field, {
            hintType: 'info',
            dismissible: false
          });
          break;
        case 'codeEditor':
          _.defaultsDeep(field, {
            language: 'none',
            lineNumbers: true
          });
          break;
        case 'tagInput':
          _.defaultsDeep(field, {
            separator: ','
          });
          break;
      }
    });
    // Populate the data model identifier field.
    setupConfObjUuidFields(this.config);
  }

  private createForm(): void {
    const controlsConfig = {};
    const allFields: Array<FormFieldConfig> = flattenFormFieldConfig(this.config);
    _.forEach(allFields, (field: FormFieldConfig) => {
      const validators: Array<ValidatorFn> = [];
      // Build the validator configuration.
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
          if ('binaryUnitInput' === field.type) {
            validators.push(CustomValidators.minBinaryUnit(field.validators.min));
          } else {
            validators.push(Validators.min(field.validators.min));
          }
        }
        if (_.isNumber(field.validators.max)) {
          if ('binaryUnitInput' === field.type) {
            validators.push(CustomValidators.maxBinaryUnit(field.validators.max));
          } else {
            validators.push(Validators.max(field.validators.max));
          }
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

  private initializeModifiers(): void {
    const allFields: FormFieldConfig[] = flattenFormFieldConfig(this.config);
    _.forEach(
      _.filter(allFields, (field) => !_.isEmpty(field.modifiers)),
      (field: FormFieldConfig) => {
        const control: AbstractControl = this.formGroup.get(field.name);
        _.forEach(field.modifiers, (modifier: FormFieldModifier) => {
          // Determine the list of form field names that this modifier
          // depends on.
          let deps: FormFieldName[] = [];
          if (_.isPlainObject(modifier.constraint) && _.isArray(modifier.deps)) {
            throw new Error('Both "constraint" and "deps" are mutually exclusive.');
          }
          if (_.isPlainObject(modifier.constraint)) {
            deps = ConstraintService.getProps(modifier.constraint);
          }
          if (_.isArray(modifier.deps)) {
            deps = _.cloneDeep(modifier.deps);
          }
          // Make sure, the field itself is not included in that list.
          _.pull(deps, field.name);
          // Subscribe to the `valueChanges` event for all involved fields.
          // If a field which is part of the constraint changes its value,
          // then the modifier is processed and applied.
          _.forEach(deps, (fieldName: string) => {
            this.subscriptions.add(
              this.formGroup.get(fieldName)?.valueChanges.subscribe(() => {
                this.doModifier(control, modifier);
              })
            );
          });
        });
      }
    );
  }

  private applyModifiers(): void {
    const allFields: FormFieldConfig[] = flattenFormFieldConfig(this.config);
    _.forEach(
      _.filter(allFields, (field) => !_.isEmpty(field.modifiers)),
      (field: FormFieldConfig) => {
        const control: AbstractControl = this.formGroup.get(field.name);
        _.forEach(field.modifiers, (modifier: FormFieldModifier) => {
          this.doModifier(control, modifier);
        });
      }
    );
  }

  private doModifier(control: AbstractControl, modifier: FormFieldModifier): void {
    const opposite = _.defaultTo(modifier.opposite, true);
    const nativeElement: HTMLElement = _.get(control, 'nativeElement');
    const formFieldElement = nativeElement && nativeElement.closest('.mat-form-field');
    // Note, use `getRawValue` here to get the latest values including
    // those of disabled form fields as well. `values` is outdated at
    // that moment because the event we are handling has not bubbled up
    // to the form yet.
    const values = _.merge({}, this.context, this.formGroup.getRawValue());
    // If there is a constraint specified, then test it, otherwise assume
    // the condition of the modifier is fulfilled. This is the case when
    // the `deps` property is specified.
    const fulfilled = _.isPlainObject(modifier.constraint)
      ? ConstraintService.test(modifier.constraint, values)
      : true;
    switch (modifier.type) {
      case 'disabled':
        if (fulfilled) {
          control.disable();
        }
        if (!fulfilled && opposite) {
          control.enable();
        }
        break;
      case 'enabled':
        if (fulfilled) {
          control.enable();
        }
        if (!fulfilled && opposite) {
          control.disable();
        }
        break;
      case 'checked':
        if (fulfilled) {
          control.setValue(true);
        }
        if (!fulfilled && opposite) {
          control.setValue(false);
        }
        break;
      case 'unchecked':
        if (fulfilled) {
          control.setValue(false);
        }
        if (!fulfilled && opposite) {
          control.setValue(true);
        }
        break;
      case 'focused':
        if (fulfilled) {
          setTimeout(() => {
            nativeElement.focus();
          });
        }
        break;
      case 'visible':
        if (!_.isUndefined(formFieldElement)) {
          if (fulfilled) {
            (formFieldElement as HTMLElement).parentElement.classList.remove('omv-display-none');
          }
          if (!fulfilled && opposite) {
            (formFieldElement as HTMLElement).parentElement.classList.add('omv-display-none');
          }
        }
        break;
      case 'hidden':
        if (!_.isUndefined(formFieldElement)) {
          if (fulfilled) {
            (formFieldElement as HTMLElement).parentElement.classList.add('omv-display-none');
          }
          if (!fulfilled && opposite) {
            (formFieldElement as HTMLElement).parentElement.classList.remove('omv-display-none');
          }
        }
        break;
      case 'value':
        if (fulfilled) {
          const value = formatDeep(modifier.typeConfig, values);
          control.setValue(value);
        }
        break;
    }
  }
}
