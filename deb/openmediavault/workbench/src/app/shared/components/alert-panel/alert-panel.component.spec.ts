import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { AlertPanelComponent } from '~/app/shared/components/alert-panel/alert-panel.component';
import { ComponentsModule } from '~/app/shared/components/components.module';
import { TestingModule } from '~/app/testing.module';

describe('AlertPanelComponent', () => {
  let component: AlertPanelComponent;
  let fixture: ComponentFixture<AlertPanelComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [ComponentsModule, TestingModule]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(AlertPanelComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
