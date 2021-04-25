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
import { EMPTY, from, Observable, Subject } from 'rxjs';
import { catchError, concatMap, map, takeUntil, tap, toArray } from 'rxjs/operators';

import { AbstractFormFieldComponent } from '~/app/core/components/limn-ui/form/components/abstract-form-field-component';
import { Icon } from '~/app/shared/enum/icon.enum';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  selector: 'omv-form-folderbrowser',
  templateUrl: './form-folderbrowser.component.html',
  styleUrls: ['./form-folderbrowser.component.scss']
})
export class FormFolderbrowserComponent
  extends AbstractFormFieldComponent
  implements OnInit, OnDestroy {
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

  protected currentPath: Array<string> = [];
  // Emits whenever the component is destroyed.
  protected readonly destroy = new Subject<void>();

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
  }

  ngOnDestroy(): void {
    this.destroy.next();
    this.destroy.complete();
  }

  open(): void {
    const control = this.formGroup.get(this.config.name);
    const path = _.trim(control.value, '/');
    const paths = path.length ? _.split(path, '/') : [];

    this.getTriggerRect();
    this.folders = [];
    this.currentPath = [];

    if (!paths.length) {
      // Display root.
      this.loadData(this.currentPath).subscribe(() => {
        this.isOpen = true;
      });
    } else {
      // Enumerate all subdirectories and stop at the first one that
      // does not exist.
      from(paths)
        .pipe(
          concatMap((name) => {
            const loadPath = this.buildPath([...this.currentPath, name]);
            return this.requestData(loadPath).pipe(
              map(() => {
                this.currentPath.push(name);
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
          this.loadData(this.currentPath).subscribe(() => {
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
      this.currentPath.pop();
    } else {
      this.currentPath.push(name);
    }
    this.loadData(this.currentPath).subscribe();
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
    const path = this.buildPath(paths);
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
  private buildPath(paths: Array<string>): string {
    return `${paths.join('/')}/`;
  }

  private buildCurrentPath(): string {
    return this.buildPath(this.currentPath);
  }

  private getTriggerRect(): void {
    this.triggerRect = this.trigger._elementRef.nativeElement.getBoundingClientRect();
  }

  private updateValue(): void {
    const control = this.formGroup.get(this.config.name);
    control.setValue(this.buildCurrentPath());
  }
}
