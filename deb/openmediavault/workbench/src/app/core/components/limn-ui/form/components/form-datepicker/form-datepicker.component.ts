import { Component } from '@angular/core';

import { AbstractFormFieldComponent } from '~/app/core/components/limn-ui/form/components/abstract-form-field-component';

@Component({
  selector: 'omv-form-datepicker',
  templateUrl: './form-datepicker.component.html',
  styleUrls: ['./form-datepicker.component.scss']
})
export class FormDatepickerComponent extends AbstractFormFieldComponent {}
