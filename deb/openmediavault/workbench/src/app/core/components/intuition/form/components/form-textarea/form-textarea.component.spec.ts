import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { TranslateModule } from '@ngx-translate/core';

import { FormTextareaComponent } from '~/app/core/components/intuition/form/components/form-textarea/form-textarea.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';

describe('FormTextareaComponent', () => {
  let component: FormTextareaComponent;
  let fixture: ComponentFixture<FormTextareaComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [IntuitionModule, NoopAnimationsModule, TranslateModule.forRoot()]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(FormTextareaComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'textarea',
      name: 'foo',
      cols: 20
    };
    const formBuilder = TestBed.inject(FormBuilder);
    component.formGroup = formBuilder.group({ foo: ['abc xyz'] });
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
