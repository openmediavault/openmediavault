import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { FormDividerComponent } from '~/app/core/components/intuition/form/components/form-divider/form-divider.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { TestingModule } from '~/app/testing.module';

describe('FormDividerComponent', () => {
  let component: FormDividerComponent;
  let fixture: ComponentFixture<FormDividerComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [IntuitionModule, TestingModule]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FormDividerComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'divider'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
