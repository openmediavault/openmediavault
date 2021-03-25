import { Component } from '@angular/core';

import { AbstractFormFieldComponent } from '~/app/core/components/limn-ui/form/components/abstract-form-field-component';

@Component({
  selector: 'omv-form-divider',
  templateUrl: './form-divider.component.html',
  styleUrls: ['./form-divider.component.scss']
})
export class FormDividerComponent extends AbstractFormFieldComponent {}
