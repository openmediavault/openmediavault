import { ComponentFixture, TestBed } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';

import { FormTagInputComponent } from '~/app/core/components/intuition/form/components/form-tag-input/form-tag-input.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { TestingModule } from '~/app/testing.module';

describe('FormTagInputComponent', () => {
  let component: FormTagInputComponent;
  let fixture: ComponentFixture<FormTagInputComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [FormTagInputComponent],
      imports: [IntuitionModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(FormTagInputComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'tagInput',
      name: 'baz'
    };
    const formBuilder = TestBed.inject(FormBuilder);
    component.formGroup = formBuilder.group({ baz: ['foo;bar;baz'] });
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
