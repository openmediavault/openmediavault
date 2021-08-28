import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';

import { FormDatepickerComponent } from '~/app/core/components/intuition/form/components/form-datepicker/form-datepicker.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';

describe('FormDatepickerComponent', () => {
  let component: FormDatepickerComponent;
  let fixture: ComponentFixture<FormDatepickerComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [IntuitionModule, NoopAnimationsModule]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(FormDatepickerComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'datePicker',
      name: 'foo'
    };
    const formBuilder = TestBed.inject(FormBuilder);
    component.formGroup = formBuilder.group({ foo: [] });
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
