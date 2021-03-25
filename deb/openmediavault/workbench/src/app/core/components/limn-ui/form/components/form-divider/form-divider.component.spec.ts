import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { FormDividerComponent } from '~/app/core/components/limn-ui/form/components/form-divider/form-divider.component';
import { LimnUiModule } from '~/app/core/components/limn-ui/limn-ui.module';

describe('FormDividerComponent', () => {
  let component: FormDividerComponent;
  let fixture: ComponentFixture<FormDividerComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [LimnUiModule]
      }).compileComponents();
    })
  );

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
