/* eslint-disable max-len */
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';
import { ToastrModule } from 'ngx-toastr';

import { FormPasswordInputComponent } from '~/app/core/components/intuition/form/components/form-password-input/form-password-input.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { TestingModule } from '~/app/testing.module';

describe('FormPasswordInputComponent', () => {
  let component: FormPasswordInputComponent;
  let fixture: ComponentFixture<FormPasswordInputComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [IntuitionModule, TestingModule, ToastrModule.forRoot()]
    }).compileComponents();
  }));

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
