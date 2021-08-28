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
import { ConnectedPosition } from '@angular/cdk/overlay';
import { ViewportRuler } from '@angular/cdk/scrolling';
import { ChangeDetectorRef, Component, OnDestroy, OnInit, ViewChild } from '@angular/core';
import { MatFormField } from '@angular/material/form-field';
import { MatSelectionListChange } from '@angular/material/list';
import * as _ from 'lodash';
import { EMPTY, from, Observable, Subject, Subscription } from 'rxjs';
import { catchError, concatMap, map, takeUntil, tap, toArray } from 'rxjs/operators';

import { AbstractFormFieldComponent } from '~/app/core/components/intuition/form/components/abstract-form-field-component';
import { Icon } from '~/app/shared/enum/icon.enum';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  selector: 'omv-form-folderbrowser',
  templateUrl: './form-folderbrowser.component.html',
  styleUrls: ['./form-folderbrowser.component.scss']
})
export class FormFolderbrowserComponent
  extends AbstractFormFieldComponent
  implements OnInit, OnDestroy
{
  @ViewChild('trigger')
  trigger: MatFormField;

  icon = Icon;
  isOpen = false;
  folders: Array<string> = [];
  triggerRect: ClientRect;
  positions: ConnectedPosition[] = [
    {
      originX: 'start',
      originY: 'top',
      overlayX: 'start',
      overlayY: 'top'
    },
    {
      originX: 'start',
      originY: 'bottom',
      overlayX: 'start',
      overlayY: 'bottom'
    }
  ];
  dirPath = '';

  protected currentPaths: Array<string> = [];
  // Emits whenever the component is destroyed.
  protected readonly destroy = new Subject<void>();

  private subscriptions = new Subscription();

  constructor(
    private changeDetectorRef: ChangeDetectorRef,
    private rpcService: RpcService,
    private viewportRuler: ViewportRuler
  ) {
    super();
  }

  ngOnInit(): void {
    super.ngOnInit();
    this.viewportRuler
      .change()
      .pipe(takeUntil(this.destroy))
      .subscribe(() => {
        if (this.isOpen) {
          this.getTriggerRect();
          this.changeDetectorRef.markForCheck();
        }
      });
    // Subscribe to changes of the 'dirRefIdField' field.
    if (this.config.dirVisible) {
      const dirRefIdControl = this.formGroup.get(this.config.dirRefIdField);
      if (dirRefIdControl) {
        this.subscriptions.add(
          dirRefIdControl.valueChanges.subscribe((value) => {
            switch (this.config.dirType) {
              case 'sharedfolder':
                this.rpcService
                  .request('ShareMgmt', 'getPath', {
                    uuid: value
                  })
                  .subscribe((res: string) => {
                    this.dirPath = _.trimEnd(res, '/') + '/';
                  });
                break;
              case 'mntent':
                this.rpcService
                  .request('FsTab', 'get', {
                    uuid: value
                  })
                  .subscribe((res: string) => {
                    this.dirPath = _.trimEnd(_.get(res, 'dir'), '/') + '/';
                  });
                break;
            }
          })
        );
      }
    }
  }

  ngOnDestroy(): void {
    this.destroy.next();
    this.destroy.complete();
    this.subscriptions.unsubscribe();
  }

  open(): void {
    const control = this.formGroup.get(this.config.name);
    const path = _.trim(control.value, '/');
    const paths = path.length ? _.split(path, '/') : [];

    this.getTriggerRect();
    this.folders = [];
    this.currentPaths = [];

    if (!paths.length) {
      // Display root.
      this.loadData(this.currentPaths).subscribe(() => {
        this.isOpen = true;
      });
    } else {
      // Enumerate all subdirectories and stop at the first one that
      // does not exist.
      from(paths)
        .pipe(
          concatMap((name) => {
            const loadPath = this.joinPaths([...this.currentPaths, name]);
            return this.requestData(loadPath).pipe(
              map(() => {
                this.currentPaths.push(name);
                return name;
              })
            );
          }),
          catchError((error) => {
            if (_.isFunction(error.preventDefault)) {
              error.preventDefault();
            }
            return EMPTY;
          }),
          toArray()
        )
        .subscribe(() => {
          this.loadData(this.currentPaths).subscribe(() => {
            this.isOpen = true;
          });
        });
    }
  }

  choose(): void {
    this.updateValue();
    this.close();
  }

  close(): void {
    this.isOpen = false;
  }

  onSelectionChange(event: MatSelectionListChange): void {
    const name = event.options[0].value;
    event.source.deselectAll();
    if ('..' === name) {
      this.currentPaths.pop();
    } else {
      this.currentPaths.push(name);
    }
    this.loadData(this.currentPaths).subscribe();
  }

  get currentPath(): string {
    return this.joinPaths(this.currentPaths);
  }

  protected sanitizeConfig() {
    super.sanitizeConfig();
    _.defaultsDeep(this.config, {
      dirVisible: false
    });
  }

  private requestData(path): Observable<any> {
    const dirRefIdControl = this.formGroup.get(this.config.dirRefIdField);
    return this.rpcService.request('FolderBrowser', 'get', {
      uuid: dirRefIdControl.value,
      type: this.config.dirType,
      path
    });
  }

  private loadData(paths: Array<string>): Observable<any> {
    const path = this.joinPaths(paths);
    return this.requestData(path).pipe(
      map((res: Array<string>) => {
        if (paths.length) {
          res.unshift('..');
        }
        return res;
      }),
      tap((folders) => {
        this.folders = folders;
      })
    );
  }

  /**
   * Get the current selected path, e.g. 'foo/bar/'.
   */
  private joinPaths(paths: Array<string>): string {
    return `${paths.join('/')}/`;
  }

  private getTriggerRect(): void {
    this.triggerRect = this.trigger._elementRef.nativeElement.getBoundingClientRect();
  }

  private updateValue(): void {
    const control = this.formGroup.get(this.config.name);
    control.setValue(this.currentPath);
  }
}
