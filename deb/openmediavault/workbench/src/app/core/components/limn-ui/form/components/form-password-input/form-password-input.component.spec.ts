/* eslint-disable max-len */
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { TranslateModule } from '@ngx-translate/core';
import { ToastrModule } from 'ngx-toastr';

import { FormPasswordInputComponent } from '~/app/core/components/limn-ui/form/components/form-password-input/form-password-input.component';
import { LimnUiModule } from '~/app/core/components/limn-ui/limn-ui.module';

describe('FormPasswordInputComponent', () => {
  let component: FormPasswordInputComponent;
  let fixture: ComponentFixture<FormPasswordInputComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [
          HttpClientTestingModule,
          LimnUiModule,
          NoopAnimationsModule,
          ToastrModule.forRoot(),
          TranslateModule.forRoot()
        ]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(FormPasswordInputComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'passwordInput',
      name: 'foo'
    };
    const formBuilder = TestBed.inject(FormBuilder);
    component.formGroup = formBuilder.group({ foo: ['secret'] });
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
