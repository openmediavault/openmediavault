/* eslint-disable max-len */
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';

import { FormSshcertSelectComponent } from '~/app/core/components/intuition/form/components/form-sshcert-select/form-sshcert-select.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';

describe('FormSshcertSelectComponent', () => {
  let component: FormSshcertSelectComponent;
  let fixture: ComponentFixture<FormSshcertSelectComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [HttpClientTestingModule, IntuitionModule, NoopAnimationsModule]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(FormSshcertSelectComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'sslCertSelect',
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
