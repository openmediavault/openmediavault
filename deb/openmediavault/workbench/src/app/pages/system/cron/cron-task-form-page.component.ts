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
export class CronTaskFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Cron',
      get: {
        method: 'get',
        params: {
          uuid: '{{ _routeParams.uuid }}'
        }
      },
      post: {
        method: 'set',
        params: {
          type: 'userdefined'
        }
      }
    },
    fields: [
      {
        type: 'confObjUuid'
      },
      {
        type: 'checkbox',
        name: 'enable',
        label: gettext('Enabled'),
        value: true
      },
      {
        type: 'select',
        name: 'execution',
        label: gettext('Time of execution'),
        value: 'exactly',
        store: {
          data: [
            ['exactly', gettext('Certain date')],
            ['hourly', gettext('Hourly')],
            ['daily', gettext('Daily')],
            ['weekly', gettext('Weekly')],
            ['monthly', gettext('Monthly')],
            ['yearly', gettext('Yearly')],
            ['reboot', gettext('At reboot')]
          ]
        }
      },
      {
        type: 'textInput',
        name: 'cronexprdesc',
        disabled: true,
        submitValue: false,
        value: '',
        modifiers: [
          {
            type: 'visible',
            constraint: { operator: 'eq', arg0: { prop: 'execution' }, arg1: 'exactly' }
          },
          {
            type: 'value',
            typeConfig:
              '{% if execution == "exactly" %}' +
              '{% set _minute = minute %}' +
              '{% set _hour = hour %}' +
              '{% set _dayofmonth = dayofmonth %}' +
              '{% if everynminute %}{% set _minute %}*/{{ minute }}{% endset %}{% endif %}' +
              '{% if everynhour %}{% set _hour %}*/{{ hour }}{% endset %}{% endif %}' +
              '{% if everyndayofmonth %}{% set _dayofmonth %}*/{{ dayofmonth }}{% endset %}{% endif %}' +
              '{{ [_minute, _hour, _dayofmonth, month, dayofweek] | join(" ") | cron2human }}' +
              '{% endif %}',
            deps: [
              'execution',
              'minute',
              'everynminute',
              'hour',
              'everynhour',
              'dayofmonth',
              'everyndayofmonth',
              'month',
              'dayofweek'
            ]
          }
        ]
      },
      {
        type: 'container',
        fields: [
          {
            type: 'select',
            name: 'minute',
            label: gettext('Minute'),
            value: ['{{ moment("H:m:ss") | split(":") | get(1) }}'],
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'ne', arg0: { prop: 'execution' }, arg1: 'exactly' }
              }
            ],
            validators: {
              requiredIf: { operator: 'eq', arg0: { prop: 'execution' }, arg1: 'exactly' },
              pattern: {
                pattern: '^(\\*|(([0-9]|[1-5][0-9]),)*([0-9]|[1-5][0-9]))$',
                errorData: gettext(
                  'The field should only contain * or a comma separated list of values.'
                )
              }
            },
            multiple: true,
            store: {
              data: [
                ['*', '*'],
                ['0', '0'],
                ['1', '1'],
                ['2', '2'],
                ['3', '3'],
                ['4', '4'],
                ['5', '5'],
                ['6', '6'],
                ['7', '7'],
                ['8', '8'],
                ['9', '9'],
                ['10', '10'],
                ['11', '11'],
                ['12', '12'],
                ['13', '13'],
                ['14', '14'],
                ['15', '15'],
                ['16', '16'],
                ['17', '17'],
                ['18', '18'],
                ['19', '19'],
                ['20', '20'],
                ['21', '21'],
                ['22', '22'],
                ['23', '23'],
                ['24', '24'],
                ['25', '25'],
                ['26', '26'],
                ['27', '27'],
                ['28', '28'],
                ['29', '29'],
                ['30', '30'],
                ['31', '31'],
                ['32', '32'],
                ['33', '33'],
                ['34', '34'],
                ['35', '35'],
                ['36', '36'],
                ['37', '37'],
                ['38', '38'],
                ['39', '39'],
                ['40', '40'],
                ['41', '41'],
                ['42', '42'],
                ['43', '43'],
                ['44', '44'],
                ['45', '45'],
                ['46', '46'],
                ['47', '47'],
                ['48', '48'],
                ['49', '49'],
                ['50', '50'],
                ['51', '51'],
                ['52', '52'],
                ['53', '53'],
                ['54', '54'],
                ['55', '55'],
                ['56', '56'],
                ['57', '57'],
                ['58', '58'],
                ['59', '59']
              ]
            }
          },
          {
            type: 'checkbox',
            name: 'everynminute',
            label: gettext('Every N minute'),
            value: false,
            modifiers: [
              {
                type: 'disabled',
                constraint: {
                  operator: 'or',
                  arg0: { operator: 'ne', arg0: { prop: 'execution' }, arg1: 'exactly' },
                  arg1: {
                    operator: 'or',
                    arg0: {
                      operator: '<>',
                      arg0: {
                        operator: 'length',
                        arg0: { prop: 'minute' }
                      },
                      arg1: 1
                    },
                    arg1: {
                      operator: 'in',
                      arg0: { value: '*' },
                      arg1: { prop: 'minute' }
                    }
                  }
                }
              },
              {
                type: 'unchecked',
                opposite: false,
                constraint: {
                  operator: '<>',
                  arg0: {
                    operator: 'length',
                    arg0: { prop: 'minute' }
                  },
                  arg1: 1
                }
              }
            ]
          }
        ]
      },
      {
        type: 'container',
        fields: [
          {
            type: 'select',
            name: 'hour',
            label: gettext('Hour'),
            value: ['{{ moment("H:m:ss") | split(":") | get(0) }}'],
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'ne', arg0: { prop: 'execution' }, arg1: 'exactly' }
              }
            ],
            validators: {
              requiredIf: { operator: 'eq', arg0: { prop: 'execution' }, arg1: 'exactly' },
              pattern: {
                pattern: '^(\\*|(([0-9]|1[0-9]|2[0-3]),)*([0-9]|1[0-9]|2[0-3]))$',
                errorData: gettext(
                  'The field should only contain * or a comma separated list of values.'
                )
              }
            },
            multiple: true,
            store: {
              data: [
                ['*', '*'],
                ['0', '0'],
                ['1', '1'],
                ['2', '2'],
                ['3', '3'],
                ['4', '4'],
                ['5', '5'],
                ['6', '6'],
                ['7', '7'],
                ['8', '8'],
                ['9', '9'],
                ['10', '10'],
                ['11', '11'],
                ['12', '12'],
                ['13', '13'],
                ['14', '14'],
                ['15', '15'],
                ['16', '16'],
                ['17', '17'],
                ['18', '18'],
                ['19', '19'],
                ['20', '20'],
                ['21', '21'],
                ['22', '22'],
                ['23', '23']
              ]
            }
          },
          {
            type: 'checkbox',
            name: 'everynhour',
            label: gettext('Every N hour'),
            value: false,
            modifiers: [
              {
                type: 'disabled',
                constraint: {
                  operator: 'or',
                  arg0: { operator: 'ne', arg0: { prop: 'execution' }, arg1: 'exactly' },
                  arg1: {
                    operator: 'or',
                    arg0: {
                      operator: '<>',
                      arg0: {
                        operator: 'length',
                        arg0: { prop: 'hour' }
                      },
                      arg1: 1
                    },
                    arg1: {
                      operator: 'in',
                      arg0: { value: '*' },
                      arg1: { prop: 'hour' }
                    }
                  }
                }
              },
              {
                type: 'unchecked',
                opposite: false,
                constraint: {
                  operator: '<>',
                  arg0: {
                    operator: 'length',
                    arg0: { prop: 'hour' }
                  },
                  arg1: 1
                }
              }
            ]
          }
        ]
      },
      {
        type: 'container',
        fields: [
          {
            type: 'select',
            name: 'dayofmonth',
            label: gettext('Day of month'),
            value: ['*'],
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'ne', arg0: { prop: 'execution' }, arg1: 'exactly' }
              }
            ],
            validators: {
              requiredIf: { operator: 'eq', arg0: { prop: 'execution' }, arg1: 'exactly' },
              pattern: {
                pattern: '^(\\*|(([1-9]|[12][0-9]|3[01]),)*([1-9]|[12][0-9]|3[01]))$',
                errorData: gettext(
                  'The field should only contain * or a comma separated list of values.'
                )
              }
            },
            multiple: true,
            store: {
              data: [
                ['*', '*'],
                ['1', '1'],
                ['2', '2'],
                ['3', '3'],
                ['4', '4'],
                ['5', '5'],
                ['6', '6'],
                ['7', '7'],
                ['8', '8'],
                ['9', '9'],
                ['10', '10'],
                ['11', '11'],
                ['12', '12'],
                ['13', '13'],
                ['14', '14'],
                ['15', '15'],
                ['16', '16'],
                ['17', '17'],
                ['18', '18'],
                ['19', '19'],
                ['20', '20'],
                ['21', '21'],
                ['22', '22'],
                ['23', '23'],
                ['24', '24'],
                ['25', '25'],
                ['26', '26'],
                ['27', '27'],
                ['28', '28'],
                ['29', '29'],
                ['30', '30'],
                ['31', '31']
              ]
            }
          },
          {
            type: 'checkbox',
            name: 'everyndayofmonth',
            label: gettext('Every N day of month'),
            value: false,
            modifiers: [
              {
                type: 'disabled',
                constraint: {
                  operator: 'or',
                  arg0: { operator: 'ne', arg0: { prop: 'execution' }, arg1: 'exactly' },
                  arg1: {
                    operator: 'or',
                    arg0: {
                      operator: '<>',
                      arg0: {
                        operator: 'length',
                        arg0: { prop: 'dayofmonth' }
                      },
                      arg1: 1
                    },
                    arg1: {
                      operator: 'in',
                      arg0: { value: '*' },
                      arg1: { prop: 'dayofmonth' }
                    }
                  }
                }
              },
              {
                type: 'unchecked',
                opposite: false,
                constraint: {
                  operator: '<>',
                  arg0: {
                    operator: 'length',
                    arg0: { prop: 'dayofmonth' }
                  },
                  arg1: 1
                }
              }
            ]
          }
        ]
      },
      {
        type: 'select',
        name: 'month',
        label: gettext('Month'),
        value: ['*'],
        modifiers: [
          {
            type: 'disabled',
            constraint: { operator: 'ne', arg0: { prop: 'execution' }, arg1: 'exactly' }
          }
        ],
        validators: {
          requiredIf: { operator: 'eq', arg0: { prop: 'execution' }, arg1: 'exactly' },
          pattern: {
            pattern: '^(\\*|(([1-9]|1[0-2]),)*([1-9]|1[0-2]))$',
            errorData: gettext(
              'The field should only contain * or a comma separated list of values.'
            )
          }
        },
        multiple: true,
        store: {
          data: [
            ['*', '*'],
            ['1', gettext('January')],
            ['2', gettext('February')],
            ['3', gettext('March')],
            ['4', gettext('April')],
            ['5', gettext('May')],
            ['6', gettext('June')],
            ['7', gettext('July')],
            ['8', gettext('August')],
            ['9', gettext('September')],
            ['10', gettext('October')],
            ['11', gettext('November')],
            ['12', gettext('December')]
          ]
        }
      },
      {
        type: 'select',
        name: 'dayofweek',
        label: gettext('Day of week'),
        value: ['*'],
        modifiers: [
          {
            type: 'disabled',
            constraint: { operator: 'ne', arg0: { prop: 'execution' }, arg1: 'exactly' }
          }
        ],
        validators: {
          requiredIf: { operator: 'eq', arg0: { prop: 'execution' }, arg1: 'exactly' },
          pattern: {
            pattern: '^(\\*|([1-7],)*[1-7])$',
            errorData: gettext(
              'The field should only contain * or a comma separated list of values.'
            )
          }
        },
        multiple: true,
        store: {
          data: [
            ['*', '*'],
            ['1', gettext('Monday')],
            ['2', gettext('Tuesday')],
            ['3', gettext('Wednesday')],
            ['4', gettext('Thursday')],
            ['5', gettext('Friday')],
            ['6', gettext('Saturday')],
            ['7', gettext('Sunday')]
          ]
        }
      },
      {
        type: 'select',
        name: 'username',
        label: gettext('User'),
        placeholder: gettext('Select an user ...'),
        value: 'root',
        valueField: 'name',
        textField: 'name',
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
        },
        validators: {
          required: true
        }
      },
      {
        type: 'textInput',
        name: 'command',
        label: gettext('Command'),
        value: '',
        validators: {
          required: true
        }
      },
      {
        type: 'checkbox',
        name: 'sendemail',
        label: gettext('Send command output via email'),
        value: false,
        hint: gettext(
          'An email message with the command output (if any produced) is send to the user who performs the job.'
        )
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
          url: '/system/cron'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/system/cron'
        }
      }
    ]
  };
}
