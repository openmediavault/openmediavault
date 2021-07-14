/* eslint-disable max-len */
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';

import { FormSshcertSelectComponent } from '~/app/core/components/limn-ui/form/components/form-sshcert-select/form-sshcert-select.component';
import { LimnUiModule } from '~/app/core/components/limn-ui/limn-ui.module';

describe('FormSshcertSelectComponent', () => {
  let component: FormSshcertSelectComponent;
  let fixture: ComponentFixture<FormSshcertSelectComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [HttpClientTestingModule, LimnUiModule, NoopAnimationsModule]
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
