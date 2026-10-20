/* eslint-disable max-len */
import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { DashboardWidgetChartComponent } from '~/app/core/components/dashboard/dashboard-widget-chart/dashboard-widget-chart.component';
import { TestingModule } from '~/app/testing.module';

describe('DashboardWidgetChartComponent', () => {
  let component: DashboardWidgetChartComponent;
  let fixture: ComponentFixture<DashboardWidgetChartComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [DashboardWidgetChartComponent],
      imports: [DashboardModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(DashboardWidgetChartComponent);
    component = fixture.componentInstance;
    component.config = {
      id: 'c2eecbb8-f40f-4110-99f1-4cb037edce68',
      type: 'chart',
      title: 'foo',
      chart: {
        type: 'doughnut',
        dataConfig: []
      }
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  it('should draw rounded doughnut segments without border', () => {
    const dataset: Record<string, any> = component.chart.config.data.datasets[0];
    expect(dataset.borderRadius).toBe(12);
    expect(dataset.spacing).toBe(2);
  });

  it('should draw rounded gauge arcs without border', () => {
    fixture = TestBed.createComponent(DashboardWidgetChartComponent);
    component = fixture.componentInstance;
    component.config = {
      id: 'c2eecbb8-f40f-4110-99f1-4cb037edce68',
      type: 'chart',
      title: 'foo',
      chart: {
        type: 'gauge',
        dataConfig: [{ label: 'CPU', prop: 'cpu', backgroundColor: '#4cd964' }]
      }
    };
    fixture.detectChanges();
    const dataset: Record<string, any> = component.chart.config.data.datasets[0];
    expect(dataset.borderRadius).toBe(12);
    expect(dataset.backgroundColor).toEqual(['#4cd964', '#0000001a']);
    expect((component.chart.config.options as any).cutout).toBe('75%');
  });
});
