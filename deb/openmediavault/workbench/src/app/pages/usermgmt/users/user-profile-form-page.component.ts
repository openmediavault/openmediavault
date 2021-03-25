import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class UserProfileFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'UserMgmt',
      get: {
        method: 'getUserByContext'
      },
      post: {
        method: 'setUserByContext'
      }
    },
    fields: [
      {
        type: 'hidden',
        name: '_readonly',
        value: false
      },
      {
        type: 'textInput',
        name: 'name',
        label: gettext('Name'),
        readonly: true
      },
      {
        type: 'textInput',
        name: 'email',
        label: gettext('Email'),
        value: '',
        validators: {
          patternType: 'email'
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
        enabledConstraint: { operator: 'falsy', arg0: { prop: '_readonly' } }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/'
        }
      }
    ]
  };
}
