import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';

import { FormSliderComponent } from '~/app/core/components/intuition/form/components/form-slider/form-slider.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';

describe('FormSliderComponent', () => {
  let component: FormSliderComponent;
  let fixture: ComponentFixture<FormSliderComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [HttpClientTestingModule, IntuitionModule, NoopAnimationsModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(FormSliderComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'slider',
      name: 'foo'
    };
    const formBuilder = TestBed.inject(FormBuilder);
    component.formGroup = formBuilder.group({ foo: [8] });
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
