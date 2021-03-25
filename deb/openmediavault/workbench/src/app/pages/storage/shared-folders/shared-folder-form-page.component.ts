import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class SharedFolderFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'ShareMgmt',
      get: {
        method: 'get',
        params: {
          uuid: '{{ _routeParams.uuid }}'
        }
      },
      post: {
        method: 'set'
      }
    },
    fields: [
      {
        type: 'confObjUuid'
      },
      {
        type: 'textInput',
        name: 'name',
        label: gettext('Name'),
        value: '',
        updateOn: 'blur',
        modifiers: [
          {
            type: 'disabled',
            constraint: { operator: 'ne', arg0: { prop: 'uuid' }, arg1: '{{ newconfobjuuid }}' }
          }
        ],
        validators: {
          required: true,
          patternType: 'shareName'
        }
      },
      {
        type: 'select',
        name: 'mntentref',
        label: gettext('Device'),
        placeholder: gettext('Select a device ...'),
        valueField: 'uuid',
        textField: 'description',
        store: {
          proxy: {
            service: 'ShareMgmt',
            get: {
              method: 'getCandidates'
            }
          },
          sorters: [
            {
              dir: 'asc',
              prop: 'description'
            }
          ]
        },
        validators: {
          required: true
        }
      },
      {
        type: 'folderBrowser',
        name: 'reldirpath',
        label: gettext('Relative path'),
        hint: gettext(
          'The relative path of the folder to share. The specified folder will be created if it does not exist.'
        ),
        value: '',
        dirType: 'mntent',
        dirRefIdField: 'mntentref',
        modifiers: [
          {
            type: 'value',
            typeConfig: '{{ name | rstrip("/") }}/',
            constraint: {
              operator: 'and',
              arg0: { operator: 'notEmpty', arg0: { prop: 'name' } },
              arg1: { operator: 'empty', arg0: { prop: 'reldirpath' } }
            }
          }
        ],
        validators: {
          required: true
        }
      },
      {
        type: 'select',
        name: 'mode',
        label: gettext('Permissions'),
        placeholder: gettext('Select permissions ...'),
        hint: gettext('The file mode of the shared folder path.'),
        value: '775',
        store: {
          data: [
            ['700', gettext('Administrator: read/write, Users: no access, Others: no access')],
            ['750', gettext('Administrator: read/write, Users: read-only, Others: no access')],
            ['770', gettext('Administrator: read/write, Users: read/write, Others: no access')],
            ['755', gettext('Administrator: read/write, Users: read-only, Others: read-only')],
            ['775', gettext('Administrator: read/write, Users: read/write, Others: read-only')],
            ['777', gettext('Everyone: read/write')]
          ]
        },
        validators: {
          required: true
        }
      },
      {
        type: 'textInput',
        name: 'comment',
        label: gettext('Comment'),
        value: ''
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/storage/shared-folders'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/storage/shared-folders'
        }
      }
    ]
  };
}
