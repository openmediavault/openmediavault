import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class FilesystemQuotaFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Quota',
      get: {
        method: 'getByTypeName',
        params: {
          uuid: '{{ _routeParams.uuid }}',
          type: '{{ _routeParams.type }}',
          name: '{{ _routeParams.name }}'
        }
      },
      post: {
        method: 'setByTypeName',
        params: {
          uuid: '{{ _routeParams.uuid }}'
        }
      }
    },
    fields: [
      {
        type: 'select',
        name: 'type',
        label: gettext('Type'),
        disabled: true,
        store: {
          data: [
            ['user', gettext('User')],
            ['group', gettext('Group')]
          ]
        }
      },
      {
        type: 'textInput',
        name: 'name',
        label: gettext('Name'),
        disabled: true
      },
      {
        type: 'numberInput',
        name: 'bhardlimit',
        label: gettext('Quota'),
        value: 0,
        validators: {
          min: 0,
          required: true
        }
      },
      {
        type: 'select',
        name: 'bunit',
        label: gettext('Unit'),
        value: 'KiB',
        store: {
          data: [
            ['KiB', 'KiB'],
            ['MiB', 'MiB'],
            ['GiB', 'GiB'],
            ['TiB', 'TiB']
          ]
        },
        validators: {
          required: true
        }
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/storage/filesystems/quota/{{ _routeParams.uuid }}'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/storage/filesystems/quota/{{ _routeParams.uuid }}'
        }
      }
    ]
  };
}
