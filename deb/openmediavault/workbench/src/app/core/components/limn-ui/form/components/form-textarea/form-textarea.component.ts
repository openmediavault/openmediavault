import { Component } from '@angular/core';
import * as _ from 'lodash';

import { AbstractFormFieldComponent } from '~/app/core/components/limn-ui/form/components/abstract-form-field-component';

@Component({
  selector: 'omv-form-textarea',
  templateUrl: './form-textarea.component.html',
  styleUrls: ['./form-textarea.component.scss']
})
export class FormTextareaComponent extends AbstractFormFieldComponent {
  protected sanitizeConfig() {
    super.sanitizeConfig();
    _.defaultsDeep(this.config, {
      rows: 4
    });
  }
}
