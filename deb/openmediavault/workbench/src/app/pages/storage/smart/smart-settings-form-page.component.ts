import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class SmartSettingsFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Smart',
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
        name: 'interval',
        label: gettext('Check interval'),
        hint: gettext(
          'Sets the interval between disk checks to N seconds, logging S.M.A.R.T. errors and changes of S.M.A.R.T. attributes.'
        ),
        value: 1800,
        validators: {
          min: 10,
          required: true
        }
      },
      {
        type: 'select',
        name: 'powermode',
        label: gettext('Power mode'),
        hint: gettext(
          'Prevent a disk from being spun-up when it is periodically polled.<ul><li>Never - Poll (check) the device regardless of its power mode. This may cause a disk which is spun-down to be spun-up when it is checked.</li><li>Sleep - Check the device unless it is in SLEEP mode.</li><li>Standby - Check the device unless it is in SLEEP or STANDBY mode. In these modes most disks are not spinning, so if you want to prevent a disk from spinning up each poll, this is probably what you want.</li><li>Idle - Check the device unless it is in SLEEP, STANDBY or IDLE mode. In the IDLE state, most disks are still spinning, so this is probably not what you want.</li></ul>'
        ),
        value: 'standby',
        store: {
          data: [
            ['never', gettext('Never')],
            ['sleep', gettext('Sleep')],
            ['standby', gettext('Standby')],
            ['idle', gettext('Idle')]
          ]
        }
      },
      {
        type: 'paragraph',
        text: gettext('Temperature monitoring')
      },
      {
        type: 'numberInput',
        name: 'tempdiff',
        label: gettext('Difference'),
        hint: gettext(
          'Report if the temperature had changed by at least N degrees Celsius since last report. Set to 0 to disable this report.'
        ),
        value: 0,
        validators: {
          min: 0,
          required: true
        }
      },
      {
        type: 'numberInput',
        name: 'tempinfo',
        label: gettext('Informal'),
        hint: gettext(
          'Report if the temperature is greater than or equal to N degrees Celsius. Set to 0 to disable this report.'
        ),
        value: 0,
        validators: {
          min: 0,
          required: true
        }
      },
      {
        type: 'numberInput',
        name: 'tempcrit',
        label: gettext('Critical'),
        hint: gettext(
          'Report if the temperature is greater than or equal to N degrees Celsius. Set to 0 to disable this report.'
        ),
        value: 0,
        validators: {
          min: 0,
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
          url: '/storage/smart'
        }
      }
    ]
  };
}
