/* eslint-disable max-len */
import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { DashboardWidgetGridComponent } from '~/app/core/components/dashboard/dashboard-widget-grid/dashboard-widget-grid.component';
import { TestingModule } from '~/app/testing.module';

describe('DashboardWidgetGridComponent', () => {
  let component: DashboardWidgetGridComponent;
  let fixture: ComponentFixture<DashboardWidgetGridComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [DashboardWidgetGridComponent],
      imports: [DashboardModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(DashboardWidgetGridComponent);
    component = fixture.componentInstance;
    component.config = {
      id: '2f06f7e5-b34f-47f7-a953-5eba2f386139',
      type: 'grid',
      title: 'bar',
      grid: {
        item: {
          title: 'baz'
        },
        store: {
          data: []
        }
      }
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
