/* eslint-disable max-len */
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';
import { ToastrModule } from 'ngx-toastr';

import { FormBinaryUnitInputComponent } from '~/app/core/components/intuition/form/components/form-binary-unit-input/form-binary-unit-input.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { PageContextService } from '~/app/core/services/page-context.service';
import { TestingModule } from '~/app/testing.module';

describe('FormBinaryUnitInputComponent', () => {
  let component: FormBinaryUnitInputComponent;
  let fixture: ComponentFixture<FormBinaryUnitInputComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [IntuitionModule, TestingModule, ToastrModule.forRoot()],
      providers: [PageContextService]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FormBinaryUnitInputComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'binaryUnitInput',
      name: 'foo',
      defaultUnit: 'B',
      fractionDigits: 0,
      validators: {
        patternType: 'binaryUnit'
      }
    };
    const formBuilder = TestBed.inject(FormBuilder);
    component.formGroup = formBuilder.group({ foo: [4711] });
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
