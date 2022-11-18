import { ComponentFixture, TestBed } from '@angular/core/testing';

import { FormHintComponent } from '~/app/core/components/intuition/form/components/form-hint/form-hint.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { TestingModule } from '~/app/testing.module';

describe('FormHintComponent', () => {
  let component: FormHintComponent;
  let fixture: ComponentFixture<FormHintComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [IntuitionModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(FormHintComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'hint',
      hintType: 'warning',
      text: 'foo bar baz'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
