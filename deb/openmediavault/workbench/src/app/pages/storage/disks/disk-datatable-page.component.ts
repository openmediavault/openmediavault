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
import { Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';

import { DatatablePageActionConfig } from '~/app/core/components/intuition/models/datatable-page-action-config.type';
import { DatatablePageConfig } from '~/app/core/components/intuition/models/datatable-page-config.type';
import { format, isUUID } from '~/app/functions.helper';
import { ModalDialogComponent } from '~/app/shared/components/modal-dialog/modal-dialog.component';
import { TaskDialogComponent } from '~/app/shared/components/task-dialog/task-dialog.component';
import { Datatable } from '~/app/shared/models/datatable.interface';
import { DialogService } from '~/app/shared/services/dialog.service';

@Component({
  template: '<omv-intuition-datatable-page [config]="this.config"></omv-intuition-datatable-page>'
})
export class DiskDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: 'c2d59665-d12a-4166-91fc-bdf4707ee539',
    autoReload: false,
    remoteSorting: true,
    remotePaging: true,
    sorters: [
      {
        dir: 'asc',
        prop: 'canonicaldevicefile'
      }
    ],
    columns: [
      { name: gettext('Device'), prop: 'canonicaldevicefile', flexGrow: 1, sortable: true },
      {
        name: gettext('Device Symlinks'),
        prop: 'devicelinks',
        flexGrow: 2,
        sortable: false,
        hidden: true,
        cellTemplateName: 'unsortedList'
      },
      {
        name: gettext('Model'),
        prop: 'model',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Serial Number'),
        prop: 'serialnumber',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('WWN'),
        prop: 'wwn',
        flexGrow: 1,
        sortable: true,
        hidden: true
      },
      {
        name: gettext('Vendor'),
        prop: 'vendor',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Capacity'),
        prop: 'size',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'binaryUnit'
      },
      {
        name: gettext('Power Mode'),
        prop: 'powermode',
        flexGrow: 1,
        sortable: true,
        hidden: true,
        cellTemplateName: 'chip',
        cellTemplateConfig: {
          /* eslint-disable @typescript-eslint/naming-convention */
          map: {
            ACTIVE: {
              value: gettext('Active'),
              class: 'omv-background-color-pair-orange'
            },
            'ACTIVE or IDLE': {
              value: gettext('Active or Idle'),
              class: 'omv-background-color-pair-orange'
            },
            IDLE: {
              value: gettext('Idle'),
              class: 'omv-background-color-pair-yellow'
            },
            STANDBY: {
              value: gettext('Standby'),
              class: 'omv-background-color-pair-blue'
            },
            SLEEP: {
              value: gettext('Sleep'),
              class: 'omv-background-color-pair-green'
            },
            UNKNOWN: {
              value: gettext('Unknown'),
              class: 'omv-background-color-pair-gray'
            },
            ERROR: {
              value: gettext('Error'),
              class: 'omv-background-color-pair-red'
            }
          }
        }
      },
      {
        name: gettext('Temperature'),
        prop: 'temperature',
        flexGrow: 1,
        sortable: true,
        hidden: true
      },
      {
        name: gettext('Hot-Pluggable'),
        prop: 'hotpluggable',
        flexGrow: 1,
        sortable: true,
        hidden: true,
        cellTemplateName: 'checkIcon'
      }
    ],
    store: {
      proxy: {
        service: 'DiskMgmt',
        get: {
          method: 'getListBg',
          task: true
        }
      },
      transform: {
        temperature: '{% if temperature %}{{ temperature }} °C{% endif %}'
      }
    },
    actions: [
      {
        template: 'edit',
        click: this.onEdit.bind(this)
      },
      {
        type: 'iconButton',
        icon: 'eraser',
        tooltip: gettext('Wipe'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1,
          constraint: [
            {
              operator: 'and',
              arg0: { operator: 'falsy', arg0: { prop: 'isroot' } },
              arg1: { operator: 'falsy', arg0: { prop: 'isreadonly' } }
            }
          ]
        },
        click: this.onWipe.bind(this)
      },
      {
        type: 'iconButton',
        icon: 'search',
        tooltip: gettext('Scan for new devices'),
        confirmationDialogConfig: {
          template: 'confirmation',
          message: gettext(
            // eslint-disable-next-line max-len
            'It may take a while to detect the new devices, thus it might be necessary to reload the table several times. Do you want to proceed?'
          )
        },
        execute: {
          type: 'request',
          request: {
            service: 'DiskMgmt',
            method: 'rescan',
            progressMessage: gettext('Scanning for new devices ...')
          }
        }
      }
    ]
  };

  constructor(
    private dialogService: DialogService,
    private router: Router
  ) {}

  onEdit(action: DatatablePageActionConfig, table: Datatable) {
    const selected = table.selection.first();
    let url: string;
    if (isUUID(_.get(selected, 'hdparm.uuid'))) {
      url = '/storage/disks/hdparm/edit/{{ hdparm.uuid }}';
    } else {
      url = '/storage/disks/hdparm/create/{{ devicefile | encodeuricomponent }}';
    }
    this.router.navigate([format(url, selected)]);
  }

  onWipe(action: DatatablePageActionConfig, table: Datatable) {
    const selected = table.selection.first();
    this.dialogService
      .open(ModalDialogComponent, {
        data: {
          template: 'confirmation-critical',
          title: gettext('Wipe'),
          message: format(
            gettext(
              'Do you really want to wipe the device {{ canonicaldevicefile }}? All data will be lost.'
            ),
            selected
          )
        }
      })
      .afterClosed()
      .subscribe((choice: boolean) => {
        if (choice) {
          this.dialogService
            .open(ModalDialogComponent, {
              data: {
                template: 'confirmation',
                title: gettext('Wipe'),
                message: gettext('Please choose the method to wipe the device.'),
                buttons: [
                  {
                    text: gettext('Cancel'),
                    dialogResult: false,
                    autofocus: true
                  },
                  {
                    text: gettext('Quick'),
                    dialogResult: 'quick',
                    class: 'omv-background-color-pair-red'
                  },
                  {
                    text: gettext('Secure'),
                    dialogResult: 'secure',
                    class: 'omv-background-color-pair-red'
                  }
                ]
              }
            })
            .afterClosed()
            .subscribe((mode: boolean | string) => {
              if (mode) {
                this.dialogService.open(TaskDialogComponent, {
                  width: '75%',
                  data: {
                    title: gettext('Wiping device'),
                    startOnInit: true,
                    request: {
                      service: 'DiskMgmt',
                      method: 'wipe',
                      params: {
                        devicefile: selected.devicefile,
                        secure: mode === 'secure'
                      }
                    }
                  }
                });
              }
            });
        }
      });
  }
}
