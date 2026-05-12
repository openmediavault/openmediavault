/* eslint-disable max-len */
import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { DashboardWidgetValueComponent } from '~/app/core/components/dashboard/dashboard-widget-value/dashboard-widget-value.component';
import { TestingModule } from '~/app/testing.module';

describe('DashboardWidgetValueComponent', () => {
  let component: DashboardWidgetValueComponent;
  let fixture: ComponentFixture<DashboardWidgetValueComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [DashboardWidgetValueComponent],
      imports: [DashboardModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(DashboardWidgetValueComponent);
    component = fixture.componentInstance;
    component.config = {
      id: '5a388783-c349-4032-9747-2482563c44dd',
      type: 'value',
      title: 'bar',
      value: {
        title: '{{ foo }} baz',
        value: 'foo {{ xyz }} bar'
      }
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
