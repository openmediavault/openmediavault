import { Component } from '@angular/core';
import * as _ from 'lodash';

import { FormSelectComponent } from '~/app/core/components/limn-ui/form/components/form-select/form-select.component';

@Component({
  selector: 'omv-form-sslcert-select',
  templateUrl: './form-sslcert-select.component.html',
  styleUrls: ['./form-sslcert-select.component.scss']
})
export class FormSslcertSelectComponent extends FormSelectComponent {
  protected sanitizeConfig() {
    super.sanitizeConfig();
    _.merge(this.config, {
      valueField: 'uuid',
      textField: 'comment',
      placeholder: 'Select a SSL certificate ...',
      store: {
        proxy: {
          service: 'CertificateMgmt',
          get: {
            method: 'getList',
            params: {
              start: 0,
              limit: -1
            }
          }
        },
        sorters: [
          {
            dir: 'asc',
            field: 'name'
          }
        ]
      }
    });
  }
}
