import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { WidgetChartComponent } from '~/app/core/components/dashboard/widgets/widget-chart/widget-chart.component';

describe('WidgetChartComponent', () => {
  let component: WidgetChartComponent;
  let fixture: ComponentFixture<WidgetChartComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [DashboardModule, HttpClientTestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(WidgetChartComponent);
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
});
