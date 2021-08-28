import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { FormComponent } from '~/app/core/components/intuition/form/form.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';

describe('FormComponent', () => {
  let component: FormComponent;
  let fixture: ComponentFixture<FormComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [IntuitionModule]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(FormComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
