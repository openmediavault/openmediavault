import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { ToastrModule } from 'ngx-toastr';

import { FormNumberInputComponent } from '~/app/core/components/limn-ui/form/components/form-number-input/form-number-input.component';
import { LimnUiModule } from '~/app/core/components/limn-ui/limn-ui.module';

describe('FormNumberInputComponent', () => {
  let component: FormNumberInputComponent;
  let fixture: ComponentFixture<FormNumberInputComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [LimnUiModule, NoopAnimationsModule, ToastrModule.forRoot()]
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
