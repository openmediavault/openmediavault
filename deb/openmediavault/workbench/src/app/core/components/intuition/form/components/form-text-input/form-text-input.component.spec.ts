import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';
import { ToastrModule } from 'ngx-toastr';

import { FormTextInputComponent } from '~/app/core/components/intuition/form/components/form-text-input/form-text-input.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { TestingModule } from '~/app/testing.module';

describe('FormTextInputComponent', () => {
  let component: FormTextInputComponent;
  let fixture: ComponentFixture<FormTextInputComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      declarations: [FormTextInputComponent],
      imports: [IntuitionModule, TestingModule, ToastrModule.forRoot()]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FormTextInputComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'textInput',
      name: 'foo'
    };
    const formBuilder = TestBed.inject(FormBuilder);
    component.formGroup = formBuilder.group({ foo: ['xyz'] });
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
