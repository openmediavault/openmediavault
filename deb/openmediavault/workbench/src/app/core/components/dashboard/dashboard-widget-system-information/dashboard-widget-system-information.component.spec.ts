/* eslint-disable max-len */
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { DashboardWidgetSystemInformationComponent } from '~/app/core/components/dashboard/dashboard-widget-system-information/dashboard-widget-system-information.component';
import { TestingModule } from '~/app/testing.module';

describe('DashboardWidgetSystemInformationComponent', () => {
  let component: DashboardWidgetSystemInformationComponent;
  let fixture: ComponentFixture<DashboardWidgetSystemInformationComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      declarations: [DashboardWidgetSystemInformationComponent],
      imports: [DashboardModule, TestingModule]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DashboardWidgetSystemInformationComponent);
    component = fixture.componentInstance;
    component.config = {
      id: '3b7d97dc-f5d3-4a5b-9300-830a0946e18a',
      type: 'system-information',
      title: 'foo'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
