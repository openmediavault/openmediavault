import { ComponentType } from '@angular/cdk/overlay';
import { Injectable } from '@angular/core';
import { MatDialog } from '@angular/material/dialog';
import { MatDialogConfig } from '@angular/material/dialog/dialog-config';
import { MatDialogRef } from '@angular/material/dialog/dialog-ref';
import * as _ from 'lodash';

@Injectable({
  providedIn: 'root'
})
export class DialogService {
  constructor(private matDialog: MatDialog) {}

  open<T, D = any, R = any>(
    component: ComponentType<T>,
    config?: MatDialogConfig<D>
  ): MatDialogRef<T, R> {
    return this.matDialog.open(
      component,
      _.defaultsDeep(config, {
        disableClose: true,
        width: '50%'
      })
    );
  }
}
