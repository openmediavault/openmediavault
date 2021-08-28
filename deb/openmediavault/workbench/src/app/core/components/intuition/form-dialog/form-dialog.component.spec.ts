import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { MAT_DIALOG_DATA, MatDialogRef } from '@angular/material/dialog';
import { RouterTestingModule } from '@angular/router/testing';
import { TranslateModule } from '@ngx-translate/core';
import { ToastrModule } from 'ngx-toastr';

import { FormDialogComponent } from '~/app/core/components/intuition/form-dialog/form-dialog.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';

describe('FormDialogComponent', () => {
  let component: FormDialogComponent;
  let fixture: ComponentFixture<FormDialogComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [
          IntuitionModule,
          HttpClientTestingModule,
          RouterTestingModule,
          ToastrModule.forRoot(),
          TranslateModule.forRoot()
        ],
        providers: [
          { provide: MAT_DIALOG_DATA, useValue: {} },
          {
            provide: MatDialogRef,
            useValue: {}
          }
        ]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(FormDialogComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
