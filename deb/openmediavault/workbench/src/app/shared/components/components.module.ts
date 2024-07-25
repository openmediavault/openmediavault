import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { TranslocoModule } from '@ngneat/transloco';
import { NgxDatatableModule } from '@siemens/ngx-datatable';

import { MaterialModule } from '~/app/material.module';
import { AlertPanelComponent } from '~/app/shared/components/alert-panel/alert-panel.component';
import { BlockUiComponent } from '~/app/shared/components/block-ui/block-ui.component';
import { DatatableComponent } from '~/app/shared/components/datatable/datatable.component';
import { DatatableActionsComponent } from '~/app/shared/components/datatable-actions/datatable-actions.component';
import { ModalDialogComponent } from '~/app/shared/components/modal-dialog/modal-dialog.component';
import { ProgressBarComponent } from '~/app/shared/components/progress-bar/progress-bar.component';
import { SubmitButtonComponent } from '~/app/shared/components/submit-button/submit-button.component';
import { TaskDialogComponent } from '~/app/shared/components/task-dialog/task-dialog.component';
import { DirectivesModule } from '~/app/shared/directives/directives.module';
import { PipesModule } from '~/app/shared/pipes/pipes.module';

@NgModule({
  declarations: [
    AlertPanelComponent,
    ModalDialogComponent,
    DatatableComponent,
    SubmitButtonComponent,
    TaskDialogComponent,
    DatatableActionsComponent,
    ProgressBarComponent,
    BlockUiComponent
  ],
  exports: [
    AlertPanelComponent,
    ModalDialogComponent,
    SubmitButtonComponent,
    DatatableComponent,
    DatatableActionsComponent,
    ProgressBarComponent,
    BlockUiComponent
  ],
  imports: [
    CommonModule,
    PipesModule,
    MaterialModule,
    NgxDatatableModule,
    TranslocoModule,
    FormsModule,
    DirectivesModule
  ]
})
export class ComponentsModule {}
