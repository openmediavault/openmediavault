/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
import { Component } from '@angular/core';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';

import { FormPageConfig } from '~/app/core/components/intuition/models/form-page-config.type';
import { BaseFormPageComponent } from '~/app/pages/base-page-component';

@Component({
  template: '<omv-intuition-form-page [config]="this.config"></omv-intuition-form-page>'
})
export class SharedFolderFormPageComponent extends BaseFormPageComponent {
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
        label: gettext('File system'),
        placeholder: gettext('Select a file system ...'),
        hint: gettext('The file system on which the shared folder is to be created.'),
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
        modifiers: [
          {
            type: 'visible',
            constraint: { operator: 'eq', arg0: { prop: 'uuid' }, arg1: '{{ newconfobjuuid }}' }
          }
        ],
        validators: {
          required: true
        }
      },
      {
        type: 'tagInput',
        name: 'comment',
        label: gettext('Tags'),
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
