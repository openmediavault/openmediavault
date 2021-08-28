/* eslint-disable max-len */
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { TranslateModule } from '@ngx-translate/core';

import { FormSharedfolderSelectComponent } from '~/app/core/components/intuition/form/components/form-sharedfolder-select/form-sharedfolder-select.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';

describe('FormSharedfolderSelectComponent', () => {
  let component: FormSharedfolderSelectComponent;
  let fixture: ComponentFixture<FormSharedfolderSelectComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [
          HttpClientTestingModule,
          IntuitionModule,
          NoopAnimationsModule,
          TranslateModule.forRoot()
        ]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(FormSharedfolderSelectComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'sharedFolderSelect',
      name: 'foo'
    };
    const formBuilder = TestBed.inject(FormBuilder);
    component.formGroup = formBuilder.group({ foo: [null] });
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
