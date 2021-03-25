import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';

import { AbstractFormFieldComponent } from '~/app/core/components/limn-ui/form/components/abstract-form-field-component';
import { Icon } from '~/app/shared/enum/icon.enum';

@Component({
  selector: 'omv-form-datatable',
  templateUrl: './form-datatable.component.html',
  styleUrls: ['./form-datatable.component.scss']
})
export class FormDatatableComponent extends AbstractFormFieldComponent {
  protected sanitizeConfig() {
    super.sanitizeConfig();
    _.defaultsDeep(this.config, {
      columnMode: 'flex',
      hasHeader: true,
      hasFooter: true,
      selectionType: 'multi',
      limit: 25,
      columns: [],
      actions: [],
      sorters: [],
      valueType: 'object'
    });
    this.config.actions.forEach((action) => {
      switch (action.template) {
        case 'add':
          _.defaultsDeep(action, {
            id: 'add',
            type: 'iconButton',
            tooltip: gettext('Add'),
            icon: Icon.add
          });
          break;
        case 'edit':
          _.defaultsDeep(action, {
            id: 'edit',
            type: 'iconButton',
            tooltip: gettext('Edit'),
            icon: Icon.edit,
            enabledConstraints: {
              minSelected: 1,
              maxSelected: 1
            }
          });
          break;
        case 'delete':
          _.defaultsDeep(action, {
            id: 'delete',
            type: 'iconButton',
            tooltip: gettext('Delete'),
            icon: Icon.delete,
            enabledConstraints: {
              minSelected: 1
            }
          });
          break;
      }
    });
  }
}
