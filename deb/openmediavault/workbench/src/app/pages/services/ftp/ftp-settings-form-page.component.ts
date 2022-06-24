/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2022 Volker Theile
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
import { IsDirtyFormPageComponent } from '~/app/pages/is-dirty-page-component';

@Component({
  template: '<omv-intuition-form-page [config]="this.config"></omv-intuition-form-page>'
})
export class FtpSettingsFormPageComponent extends IsDirtyFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'FTP',
      get: {
        method: 'getSettings'
      },
      post: {
        method: 'setSettings'
      }
    },
    fields: [
      /* eslint-disable max-len */
      {
        type: 'checkbox',
        name: 'enable',
        label: gettext('Enabled'),
        value: false
      },
      {
        type: 'numberInput',
        name: 'port',
        label: gettext('Port'),
        value: 21,
        validators: {
          required: true,
          min: 1,
          max: 65535,
          patternType: 'port'
        }
      },
      {
        type: 'numberInput',
        name: 'maxclients',
        label: gettext('Max. clients'),
        hint: gettext('Maximum number of simultaneous clients.'),
        value: 5,
        validators: {
          required: true,
          min: 0,
          patternType: 'integer'
        }
      },
      {
        type: 'numberInput',
        name: 'maxconnectionsperhost',
        label: gettext('Max. connections per host'),
        hint: gettext('Maximum number of connections per IP (0 = unlimited).'),
        value: 2,
        validators: {
          required: true,
          min: 0,
          patternType: 'integer'
        }
      },
      {
        type: 'numberInput',
        name: 'maxloginattempts',
        label: gettext('Max. login attempts'),
        hint: gettext('Maximum number of allowed password attempts before disconnection.'),
        value: 1,
        validators: {
          required: true,
          min: 0,
          patternType: 'integer'
        }
      },
      {
        type: 'numberInput',
        name: 'timeoutidle',
        label: gettext('Timeout'),
        hint: gettext(
          'Maximum idle time in seconds. Setting idle timeout to 0 disables the idle timer completely (clients can stay connected for ever, without sending data).'
        ),
        value: 1200,
        validators: {
          required: true,
          min: 0,
          patternType: 'integer'
        }
      },
      {
        type: 'checkbox',
        name: 'anonymous',
        label: gettext('Anonymous FTP'),
        value: false
      },
      {
        type: 'textarea',
        name: 'displaylogin',
        label: gettext('Welcome message'),
        value: '',
        hint: gettext(
          'The welcome message which will be displayed to the user when they initially login.'
        )
      },
      {
        type: 'divider',
        title: gettext('Home directories')
      },
      {
        type: 'checkbox',
        name: 'homesenable',
        label: gettext('Enabled'),
        hint: gettext('Display the home directory of the user in the browse list.'),
        value: false
      },
      {
        type: 'divider',
        title: gettext('Advanced settings')
      },
      {
        type: 'checkbox',
        name: 'rootlogin',
        label: gettext('Permit root login'),
        hint: gettext('Specifies whether it is allowed to login as superuser directly.'),
        value: false
      },
      {
        type: 'checkbox',
        name: 'requirevalidshell',
        label: gettext('Require valid shell'),
        hint: gettext('Deny logins which do not have a valid shell.'),
        value: true
      },
      {
        type: 'checkbox',
        name: 'limittransferrate',
        label: gettext('Bandwidth restriction'),
        hint: gettext('Use the following bandwidth restriction.'),
        value: false
      },
      {
        type: 'container',
        fields: [
          {
            type: 'numberInput',
            name: 'maxuptransferrate',
            label: gettext('Maximum upload rate (KiB/s)'),
            hint: gettext('0 KiB/s means unlimited.'),
            value: 0,
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'falsy', arg0: { prop: 'limittransferrate' } }
              }
            ],
            validators: {
              min: 0,
              patternType: 'integer',
              required: true
            }
          },
          {
            type: 'numberInput',
            name: 'maxdowntransferrate',
            label: gettext('Maximum download rate (KiB/s)'),
            hint: gettext('0 KiB/s means unlimited.'),
            value: 0,
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'falsy', arg0: { prop: 'limittransferrate' } }
              }
            ],
            validators: {
              min: 0,
              patternType: 'integer',
              required: true
            }
          }
        ]
      },
      {
        type: 'checkbox',
        name: 'usepassiveports',
        label: gettext('Passive FTP'),
        hint: gettext(
          'In some cases you have to specify passive ports range to by-pass firewall limitations. Passive ports restricts the range of ports from which the server will select when sent the PASV command from a client. The server will randomly choose a number from within the specified range until an open port is found. The port range selected must be in the non-privileged range (eg. greater than or equal to 1024). It is strongly recommended that the chosen range be large enough to handle many simultaneous passive connections (for example, 49152-65534, the IANA-registered ephemeral port range).'
        ),
        value: false
      },
      {
        type: 'container',
        fields: [
          {
            type: 'numberInput',
            name: 'minpassiveports',
            label: gettext('Min. passive port'),
            value: 49152,
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'falsy', arg0: { prop: 'usepassiveports' } }
              }
            ],
            validators: {
              patternType: 'port',
              min: 1025,
              max: 65535,
              required: true
            }
          },
          {
            type: 'numberInput',
            name: 'maxpassiveports',
            label: gettext('Max. passive port'),
            value: 65534,
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'falsy', arg0: { prop: 'usepassiveports' } }
              }
            ],
            validators: {
              patternType: 'port',
              min: 1025,
              max: 65535,
              required: true
            }
          }
        ]
      },
      {
        type: 'container',
        fields: [
          {
            type: 'textInput',
            name: 'masqueradeaddress',
            label: gettext('Masquerade address'),
            value: '',
            hint: gettext(
              'If your host is acting as a NAT gateway or port forwarder for the server, this option is useful in order to allow passive tranfers to work. You have to use your public address and opening the passive ports used on your firewall as well.'
            ),
            validators: {
              patternType: 'domainNameIp'
            }
          },
          {
            type: 'numberInput',
            label: gettext('Refresh time'),
            name: 'dynmasqrefresh',
            hint: gettext(
              'Specifies the amount of time, in seconds, between checking and updating the masquerade address by resolving the IP address. Set this value to 0 to disable this option.'
            ),
            value: 0,
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'z', arg0: { prop: 'masqueradeaddress' } }
              }
            ],
            validators: {
              min: 0,
              patternType: 'integer',
              required: true
            }
          }
        ]
      },
      {
        type: 'checkbox',
        name: 'allowforeignaddress',
        label: gettext('FXP'),
        hint: gettext(
          'FXP allows transfers between two remote servers without any file data going to the client asking for the transfer.'
        ),
        value: false
      },
      {
        type: 'checkbox',
        name: 'allowrestart',
        label: gettext('Resume'),
        hint: gettext('Allow clients to resume interrupted uploads and downloads.'),
        value: false
      },
      {
        type: 'checkbox',
        name: 'identlookups',
        label: gettext('Ident protocol'),
        hint: gettext(
          'When a client initially connects to the server the ident protocol is used to attempt to identify the remote user name.'
        ),
        value: false
      },
      {
        type: 'checkbox',
        name: 'usereversedns',
        label: gettext('Reverse DNS lookup'),
        hint: gettext(
          "Enable reverse DNS lookup performed on the remote host's IP address for incoming active mode data connections and outgoing passive mode data connections."
        ),
        value: false
      },
      {
        type: 'checkbox',
        name: 'transferlog',
        label: gettext('Transfer log'),
        value: false
      },
      {
        type: 'textarea',
        name: 'extraoptions',
        label: gettext('Extra options'),
        hint: gettext(
          "Please check the <a href='http://www.proftpd.org/docs/directives/configuration_full.html' target='_blank'>manual page</a> for more details."
        ),
        value: ''
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
          url: '/services/ftp'
        }
      }
    ]
  };
}
