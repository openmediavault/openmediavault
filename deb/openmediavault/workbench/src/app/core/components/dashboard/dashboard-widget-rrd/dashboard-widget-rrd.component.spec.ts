import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { DashboardWidgetRrdComponent } from '~/app/core/components/dashboard/dashboard-widget-rrd/dashboard-widget-rrd.component';
import { TestingModule } from '~/app/testing.module';

describe('DashboardWidgetRrdComponent', () => {
  let component: DashboardWidgetRrdComponent;
  let fixture: ComponentFixture<DashboardWidgetRrdComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      declarations: [DashboardWidgetRrdComponent],
      imports: [DashboardModule, TestingModule]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DashboardWidgetRrdComponent);
    component = fixture.componentInstance;
    component.config = {
      id: 'b7e2e07e-4b61-4b39-86f6-df217edd91c7',
      type: 'rrd',
      title: 'abc'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
