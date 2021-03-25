import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class GroupFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'UserMgmt',
      get: {
        method: 'getGroup',
        params: {
          name: '{{ _routeParams.name }}'
        }
      },
      post: {
        method: 'setGroup'
      }
    },
    fields: [
      {
        type: 'textInput',
        name: 'name',
        label: gettext('Name'),
        disabled: '{{ _routeConfig.data.editing | toboolean }}',
        value: '{{ _routeParams.name }}',
        validators: {
          required: true,
          patternType: 'groupName'
        }
      },
      {
        type: 'select',
        name: 'members',
        label: gettext('Members'),
        placeholder: gettext('Select members ...'),
        multiple: true,
        valueField: 'name',
        textField: 'name',
        value: [],
        store: {
          proxy: {
            service: 'UserMgmt',
            get: {
              method: 'enumerateAllUsers'
            }
          },
          sorters: [
            {
              dir: 'asc',
              prop: 'name'
            }
          ]
        }
      },
      {
        type: 'textInput',
        name: 'comment',
        label: gettext('Comment'),
        value: '',
        validators: {
          maxLength: 65
        }
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/usermgmt/groups'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/usermgmt/groups'
        }
      }
    ]
  };
}
