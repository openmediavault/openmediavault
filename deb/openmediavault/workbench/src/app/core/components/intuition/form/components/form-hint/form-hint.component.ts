import { Component } from '@angular/core';

import { AbstractFormFieldComponent } from '~/app/core/components/intuition/form/components/abstract-form-field-component';

@Component({
  selector: 'omv-form-hint',
  templateUrl: './form-hint.component.html',
  styleUrls: ['./form-hint.component.scss']
})
export class FormHintComponent extends AbstractFormFieldComponent {}
