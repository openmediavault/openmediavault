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
export class GeneralNetworkFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Network',
      get: {
        method: 'getGeneralSettings'
      },
      post: {
        method: 'setGeneralSettings'
      }
    },
    fields: [
      {
        type: 'textInput',
        name: 'hostname',
        label: gettext('Hostname'),
        hint: gettext('The hostname is a label that identifies the system to the network.'),
        value: '',
        validators: {
          patternType: 'hostName',
          required: true
        }
      },
      {
        type: 'textInput',
        name: 'domainname',
        label: gettext('Domain name'),
        hint: gettext(
          'The domain name of the system. If your system is part of a private network without a registered domain, use <em>internal</em> as suggested by <a href="https://datatracker.ietf.org/doc/html/draft-davies-internal-tld-01" target="_blank">ICANN</a>.'
        ),
        value: '',
        validators: {
          patternType: 'domainName',
          required: true
        }
      }
    ],
    buttons: [
      {
        template: 'submit'
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/network'
        }
      }
    ]
  };
}
