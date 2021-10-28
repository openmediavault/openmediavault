/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/intuition/models/form-page-config.type';

@Component({
  template: '<omv-intuition-form-page [config]="this.config"></omv-intuition-form-page>'
})
export class FtpBanRuleFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'FTP',
      get: {
        method: 'getModBanRule',
        params: {
          uuid: '{{ _routeParams.uuid }}'
        }
      },
      post: {
        method: 'setModBanRule'
      }
    },
    fields: [
      {
        type: 'confObjUuid'
      },
      {
        type: 'select',
        name: 'event',
        label: gettext('Event'),
        hint: gettext('This rule is triggered whenever the selected event directive occurs.'),
        value: 'MaxConnectionsPerHost',
        store: {
          data: [
            'AnonRejectPasswords',
            'ClientConnectRate',
            'MaxClientsPerClass',
            'MaxClientsPerHost',
            'MaxClientsPerUser',
            'MaxConnectionsPerHost',
            'MaxHostsPerUser',
            'MaxLoginAttempts',
            'TimeoutIdle',
            'TimeoutNoTransfer'
          ]
        }
      },
      {
        type: 'numberInput',
        name: 'occurrence',
        label: gettext('Occurrence'),
        hint: gettext(
          'This parameter says that if N occurrences of the event happen within the given time interval, then a ban is automatically added.'
        ),
        value: 2,
        validators: {
          required: true,
          min: 1,
          patternType: 'integer'
        }
      },
      {
        type: 'textInput',
        name: 'timeinterval',
        label: gettext('Time interval'),
        hint: gettext(
          'Specifies the time interval in hh:mm:ss in which the given number of occurrences must happen to add the ban.'
        ),
        value: '00:30:00',
        validators: {
          required: true,
          patternType: 'time',
          maxLength: 81
        }
      },
      {
        type: 'textInput',
        name: 'expire',
        label: gettext('Expire'),
        hint: gettext('Specifies the time in hh:mm:ss after which the ban expires.'),
        value: '00:10:00',
        validators: {
          required: true,
          patternType: 'time',
          maxLength: 81
        }
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/services/ftp/ban-rules'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/services/ftp/ban-rules'
        }
      }
    ]
  };
}
