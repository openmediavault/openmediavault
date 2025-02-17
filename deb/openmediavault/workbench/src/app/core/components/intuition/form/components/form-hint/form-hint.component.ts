import { Component, ViewEncapsulation } from '@angular/core';

import { AbstractFormFieldComponent } from '~/app/core/components/intuition/form/components/abstract-form-field-component';
import { PageContextService } from '~/app/shared/services/pagecontext-service';

@Component({
  selector: 'omv-form-hint',
  templateUrl: './form-hint.component.html',
  styleUrls: ['./form-hint.component.scss'],
  encapsulation: ViewEncapsulation.None,
  providers: [PageContextService]
})
export class FormHintComponent extends AbstractFormFieldComponent {}
