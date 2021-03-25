import { Component } from '@angular/core';
import * as _ from 'lodash';

import { AbstractFormFieldComponent } from '~/app/core/components/limn-ui/form/components/abstract-form-field-component';

@Component({
  selector: 'omv-form-paragraph',
  templateUrl: './form-paragraph.component.html',
  styleUrls: ['./form-paragraph.component.scss']
})
export class FormParagraphComponent extends AbstractFormFieldComponent {
  protected sanitizeConfig() {
    super.sanitizeConfig();
    _.defaultsDeep(this.config, {
      hasDivider: true
    });
  }
}
