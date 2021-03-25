import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { FormComponent } from '~/app/core/components/limn-ui/form/form.component';
import { LimnUiModule } from '~/app/core/components/limn-ui/limn-ui.module';

describe('FormComponent', () => {
  let component: FormComponent;
  let fixture: ComponentFixture<FormComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [LimnUiModule]
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
