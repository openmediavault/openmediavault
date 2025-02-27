import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';

import { FormCheckboxComponent } from '~/app/core/components/intuition/form/components/form-checkbox/form-checkbox.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { PageContextService } from '~/app/core/services/page-context.service';
import { TestingModule } from '~/app/testing.module';

describe('FormCheckboxComponent', () => {
  let component: FormCheckboxComponent;
  let fixture: ComponentFixture<FormCheckboxComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [IntuitionModule, TestingModule],
      providers: [PageContextService]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FormCheckboxComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'checkbox',
      name: 'foo'
    };
    const formBuilder = TestBed.inject(FormBuilder);
    component.formGroup = formBuilder.group({ foo: [false] });
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
