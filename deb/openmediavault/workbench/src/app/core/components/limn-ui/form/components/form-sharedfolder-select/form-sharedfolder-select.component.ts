import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';

import { FormSelectComponent } from '~/app/core/components/limn-ui/form/components/form-select/form-select.component';

@Component({
  selector: 'omv-form-sharedfolder-select',
  templateUrl: './form-sharedfolder-select.component.html',
  styleUrls: ['./form-sharedfolder-select.component.scss']
})
export class FormSharedfolderSelectComponent extends FormSelectComponent {
  protected sanitizeConfig() {
    super.sanitizeConfig();
    _.merge(this.config, {
      valueField: 'uuid',
      textField: 'description',
      placeholder: gettext('Select a shared folder ...'),
      store: {
        proxy: {
          service: 'ShareMgmt',
          get: {
            method: 'enumerateSharedFolders'
          }
        },
        sorters: [
          {
            dir: 'asc',
            prop: 'name'
          }
        ]
      }
    });
  }
}
