import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { ToastrModule } from 'ngx-toastr';

import { FormNumberInputComponent } from '~/app/core/components/intuition/form/components/form-number-input/form-number-input.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';

describe('FormNumberInputComponent', () => {
  let component: FormNumberInputComponent;
  let fixture: ComponentFixture<FormNumberInputComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [IntuitionModule, NoopAnimationsModule, ToastrModule.forRoot()]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(FormNumberInputComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'numberInput',
      name: 'foo'
    };
    const formBuilder = TestBed.inject(FormBuilder);
    component.formGroup = formBuilder.group({ foo: [4711] });
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
