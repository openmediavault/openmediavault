import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';
import { ToastrModule } from 'ngx-toastr';

import { FormIconButtonComponent } from '~/app/core/components/intuition/form/components/form-icon-button/form-icon-button.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { TestingModule } from '~/app/testing.module';

describe('FormIconButtonComponent', () => {
  let component: FormIconButtonComponent;
  let fixture: ComponentFixture<FormIconButtonComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [IntuitionModule, TestingModule, ToastrModule.forRoot()]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FormIconButtonComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'iconButton',
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
