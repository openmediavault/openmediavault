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
export class RsyncTaskFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Rsync',
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
      /* eslint-disable max-len */
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
        name: 'type',
        label: gettext('Type'),
        value: 'local',
        store: {
          data: [
            ['local', gettext('Local')],
            ['remote', gettext('Remote')]
          ]
        }
      },
      {
        type: 'select',
        name: 'mode',
        label: gettext('Mode'),
        value: 'push',
        store: {
          data: [
            ['push', gettext('Push')],
            ['pull', gettext('Pull')]
          ]
        },
        modifiers: [
          {
            type: 'visible',
            constraint: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'remote' }
          }
        ]
      },
      {
        type: 'sharedFolderSelect',
        name: 'srcsharedfolderref',
        hasEmptyOption: true,
        value: '',
        label: gettext('Source shared folder'),
        modifiers: [
          {
            type: 'hidden',
            constraint: {
              operator: 'not',
              arg0: {
                operator: 'or',
                arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'local' },
                arg1: {
                  operator: 'and',
                  arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'remote' },
                  arg1: { operator: 'eq', arg0: { prop: 'mode' }, arg1: 'push' }
                }
              }
            }
          }
        ],
        validators: {
          requiredIf: {
            operator: 'or',
            arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'local' },
            arg1: {
              operator: 'and',
              arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'remote' },
              arg1: { operator: 'eq', arg0: { prop: 'mode' }, arg1: 'push' }
            }
          }
        }
      },
      {
        type: 'textInput',
        name: 'srcuri',
        label: gettext('Source server'),
        value: '',
        hint: gettext(
          'The source remote server, e.g. [USER@]HOST:SRC, [USER@]HOST::SRC or rsync://[USER@]HOST[:PORT]/SRC.'
        ),
        modifiers: [
          {
            type: 'hidden',
            constraint: {
              operator: 'not',
              arg0: {
                operator: 'and',
                arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'remote' },
                arg1: { operator: 'eq', arg0: { prop: 'mode' }, arg1: 'pull' }
              }
            }
          }
        ],
        validators: {
          requiredIf: {
            operator: 'and',
            arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'remote' },
            arg1: { operator: 'eq', arg0: { prop: 'mode' }, arg1: 'pull' }
          }
        }
      },
      {
        type: 'sharedFolderSelect',
        name: 'destsharedfolderref',
        hasEmptyOption: true,
        value: '',
        label: gettext('Destination shared folder'),
        modifiers: [
          {
            type: 'hidden',
            constraint: {
              operator: 'not',
              arg0: {
                operator: 'or',
                arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'local' },
                arg1: {
                  operator: 'and',
                  arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'remote' },
                  arg1: { operator: 'eq', arg0: { prop: 'mode' }, arg1: 'pull' }
                }
              }
            }
          }
        ],
        validators: {
          requiredIf: {
            operator: 'or',
            arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'local' },
            arg1: {
              operator: 'and',
              arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'remote' },
              arg1: { operator: 'eq', arg0: { prop: 'mode' }, arg1: 'pull' }
            }
          }
        }
      },
      {
        type: 'textInput',
        name: 'desturi',
        label: gettext('Destination server'),
        value: '',
        hint: gettext(
          'The destination remote server, e.g. [USER@]HOST:DEST, [USER@]HOST::DEST or rsync://[USER@]HOST[:PORT]/DEST.'
        ),
        modifiers: [
          {
            type: 'hidden',
            constraint: {
              operator: 'not',
              arg0: {
                operator: 'and',
                arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'remote' },
                arg1: { operator: 'eq', arg0: { prop: 'mode' }, arg1: 'push' }
              }
            }
          }
        ],
        validators: {
          requiredIf: {
            operator: 'and',
            arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'remote' },
            arg1: { operator: 'eq', arg0: { prop: 'mode' }, arg1: 'push' }
          }
        }
      },
      {
        type: 'select',
        name: 'authentication',
        label: gettext('Authentication'),
        placeholder: gettext('Select an authentication mode ...'),
        value: 'password',
        store: {
          data: [
            ['password', gettext('Password')],
            ['pubkey', gettext('Public key')]
          ]
        },
        modifiers: [
          {
            type: 'hidden',
            constraint: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'local' }
          }
        ]
      },

      {
        type: 'numberInput',
        name: 'sshport',
        label: gettext('SSH port'),
        value: 22,
        modifiers: [
          {
            type: 'hidden',
            constraint: {
              operator: 'or',
              arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'local' },
              arg1: { operator: 'eq', arg0: { prop: 'authentication' }, arg1: 'password' }
            }
          }
        ],
        validators: {
          min: 1,
          max: 65535,
          requiredIf: {
            operator: 'and',
            arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'remote' },
            arg1: { operator: 'eq', arg0: { prop: 'authentication' }, arg1: 'pubkey' }
          }
        }
      },
      {
        type: 'sshCertSelect',
        name: 'sshcertificateref',
        hasEmptyOption: true,
        value: '',
        label: gettext('SSH certificate'),
        hint: gettext('The SSH certificate used for authentication.'),
        modifiers: [
          {
            type: 'hidden',
            constraint: {
              operator: 'or',
              arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'local' },
              arg1: { operator: 'eq', arg0: { prop: 'authentication' }, arg1: 'password' }
            }
          }
        ],
        validators: {
          requiredIf: {
            operator: 'and',
            arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'remote' },
            arg1: { operator: 'eq', arg0: { prop: 'authentication' }, arg1: 'pubkey' }
          }
        }
      },
      {
        type: 'passwordInput',
        name: 'password',
        label: gettext('Password'),
        hint: gettext(
          'The password that is used for access via rsync daemon. Note, this is not used for remote shell transport such as SSH.'
        ),
        value: '',
        autocomplete: 'new-password',
        modifiers: [
          {
            type: 'hidden',
            constraint: {
              operator: 'or',
              arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'local' },
              arg1: { operator: 'eq', arg0: { prop: 'authentication' }, arg1: 'pubkey' }
            }
          }
        ],
        validators: {
          requiredIf: {
            operator: 'and',
            arg0: { operator: 'eq', arg0: { prop: 'type' }, arg1: 'remote' },
            arg1: { operator: 'eq', arg0: { prop: 'authentication' }, arg1: 'password' }
          }
        }
      },
      {
        type: 'textInput',
        name: 'cronexprdesc',
        label: gettext('Time of execution'),
        disabled: true,
        submitValue: false,
        value: '',
        modifiers: [
          {
            type: 'value',
            typeConfig:
              '{% set _minute = minute %}' +
              '{% set _hour = hour %}' +
              '{% set _dayofmonth = dayofmonth %}' +
              '{% if everynminute %}{% set _minute %}*/{{ minute }}{% endset %}{% endif %}' +
              '{% if everynhour %}{% set _hour %}*/{{ hour }}{% endset %}{% endif %}' +
              '{% if everyndayofmonth %}{% set _dayofmonth %}*/{{ dayofmonth }}{% endset %}{% endif %}' +
              '{{ [_minute, _hour, _dayofmonth, month, dayofweek] | join(" ") | cron2human }}',
            deps: [
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
            value: ['{{ moment("m") }}'],
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
            },
            multiple: true,
            validators: {
              required: true,
              pattern: {
                pattern: '^(\\*|(([0-9]|[1-5][0-9]),)*([0-9]|[1-5][0-9]))$',
                errorData: gettext(
                  'The field should only contain * or a comma separated list of values.'
                )
              }
            }
          },
          {
            type: 'checkbox',
            name: 'everynminute',
            label: gettext('Every N minute'),
            value: false,
            modifiers: [
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
              },
              {
                type: 'disabled',
                constraint: {
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
            value: ['{{ moment("H") }}'],
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
            },
            multiple: true,
            validators: {
              required: true,
              pattern: {
                pattern: '^(\\*|(([0-9]|1[0-9]|2[0-3]),)*([0-9]|1[0-9]|2[0-3]))$',
                errorData: gettext(
                  'The field should only contain * or a comma separated list of values.'
                )
              }
            }
          },
          {
            type: 'checkbox',
            name: 'everynhour',
            label: gettext('Every N hour'),
            value: false,
            modifiers: [
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
              },
              {
                type: 'disabled',
                constraint: {
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
            },
            multiple: true,
            validators: {
              required: true,
              pattern: {
                pattern: '^(\\*|(([1-9]|[12][0-9]|3[01]),)*([1-9]|[12][0-9]|3[01]))$',
                errorData: gettext(
                  'The field should only contain * or a comma separated list of values.'
                )
              }
            }
          },
          {
            type: 'checkbox',
            name: 'everyndayofmonth',
            label: gettext('Every N day of month'),
            value: false,
            modifiers: [
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
              },
              {
                type: 'disabled',
                constraint: {
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
            ]
          }
        ]
      },
      {
        type: 'select',
        name: 'month',
        label: gettext('Month'),
        value: ['*'],
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
        },
        multiple: true,
        validators: {
          required: true,
          pattern: {
            pattern: '^(\\*|(([1-9]|1[0-2]),)*([1-9]|1[0-2]))$',
            errorData: gettext(
              'The field should only contain * or a comma separated list of values.'
            )
          }
        }
      },
      {
        type: 'select',
        name: 'dayofweek',
        label: gettext('Day of week'),
        value: ['*'],
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
        },
        multiple: true,
        validators: {
          required: true,
          pattern: {
            pattern: '^(\\*|([1-7],)*[1-7])$',
            errorData: gettext(
              'The field should only contain * or a comma separated list of values.'
            )
          }
        }
      },
      {
        type: 'checkbox',
        name: 'sendemail',
        label: gettext('Send command output via email'),
        value: false,
        hint: gettext(
          'An email message with the command output (if any produced) is send to the administrator.'
        )
      },
      {
        type: 'checkbox',
        name: 'optiondryrun',
        label: gettext('Trial run'),
        hint: gettext('Perform a trial run with no changes made'),
        value: false
      },
      {
        type: 'checkbox',
        name: 'optionquiet',
        label: gettext('Suppress non-error messages'),
        value: false
      },
      {
        type: 'checkbox',
        name: 'optionarchive',
        label: gettext('Archive mode'),
        value: true,
        modifiers: [
          {
            type: 'unchecked',
            opposite: false,
            constraint: { operator: 'falsy', arg0: { prop: 'optionrecursive' } }
          },
          {
            type: 'unchecked',
            opposite: false,
            constraint: { operator: 'falsy', arg0: { prop: 'optionperms' } }
          },
          {
            type: 'unchecked',
            opposite: false,
            constraint: { operator: 'falsy', arg0: { prop: 'optiontimes' } }
          },
          {
            type: 'unchecked',
            opposite: false,
            constraint: { operator: 'falsy', arg0: { prop: 'optiongroup' } }
          },
          {
            type: 'unchecked',
            opposite: false,
            constraint: { operator: 'falsy', arg0: { prop: 'optionowner' } }
          }
        ]
      },
      {
        type: 'checkbox',
        name: 'optionrecursive',
        label: gettext('Recurse into directories'),
        value: true,
        modifiers: [
          {
            type: 'checked',
            opposite: false,
            constraint: { operator: 'truthy', arg0: { prop: 'optionarchive' } }
          }
        ]
      },
      {
        type: 'checkbox',
        name: 'optionperms',
        label: gettext('Preserve permissions'),
        hint: gettext('Set the destination permissions to be the same as the source permissions.'),
        value: true,
        modifiers: [
          {
            type: 'checked',
            opposite: false,
            constraint: { operator: 'truthy', arg0: { prop: 'optionarchive' } }
          }
        ]
      },
      {
        type: 'checkbox',
        name: 'optiontimes',
        label: gettext('Preserve modification times'),
        hint: gettext(
          'Transfer modification times along with the files and update them on the remote system.'
        ),
        value: true,
        modifiers: [
          {
            type: 'checked',
            opposite: false,
            constraint: { operator: 'truthy', arg0: { prop: 'optionarchive' } }
          }
        ]
      },
      {
        type: 'checkbox',
        name: 'optiongroup',
        label: gettext('Preserve group'),
        hint: gettext('Set the group of the destination file to be the same as the source file.'),
        value: true,
        modifiers: [
          {
            type: 'checked',
            opposite: false,
            constraint: { operator: 'truthy', arg0: { prop: 'optionarchive' } }
          }
        ]
      },
      {
        type: 'checkbox',
        name: 'optionowner',
        label: gettext('Preserve owner'),
        hint: gettext(
          'Set the owner of the destination file to be the same as the source file, but only if the receiving rsync is being run as the super-user.'
        ),
        value: true,
        modifiers: [
          {
            type: 'checked',
            opposite: false,
            constraint: { operator: 'truthy', arg0: { prop: 'optionarchive' } }
          }
        ]
      },
      {
        type: 'checkbox',
        name: 'optioncompress',
        label: gettext('Compress'),
        hint: gettext('Compress file data during the transfer.'),
        value: false
      },
      {
        type: 'checkbox',
        name: 'optionacls',
        label: gettext('Preserve ACLs'),
        hint: gettext('Update the destination ACLs to be the same as the source ACLs.'),
        value: false
      },
      {
        type: 'checkbox',
        name: 'optionxattrs',
        label: gettext('Preserve extended attributes'),
        hint: gettext(
          'Update the destination extended attributes to be the same as the local ones.'
        ),
        value: false
      },
      {
        type: 'checkbox',
        name: 'optionpartial',
        label: gettext('Keep partially transferred files'),
        hint: gettext(
          'Enable this option to keep partially transferred files, otherwise they will be deleted if the transfer is interrupted.'
        ),
        value: false
      },
      {
        type: 'checkbox',
        name: 'optiondelete',
        label: gettext('Delete'),
        hint: gettext("Delete files on the receiving side that don't exist on sender."),
        value: false
      },
      {
        type: 'textInput',
        name: 'extraoptions',
        label: gettext('Extra options'),
        hint: gettext(
          'Please check the <a href="http://www.samba.org/ftp/rsync/rsync.html" target="_blank">manual page</a> for more details.'
        ),
        value: ''
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
          url: '/services/rsync/tasks'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/services/rsync/tasks'
        }
      }
    ]
  };
}
