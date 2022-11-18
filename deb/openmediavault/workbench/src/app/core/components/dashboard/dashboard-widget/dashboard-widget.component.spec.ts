import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { DashboardWidgetComponent } from '~/app/core/components/dashboard/dashboard-widget/dashboard-widget.component';
import { TestingModule } from '~/app/testing.module';

describe('DashboardWidgetComponent', () => {
  let component: DashboardWidgetComponent;
  let fixture: ComponentFixture<DashboardWidgetComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [DashboardWidgetComponent],
      imports: [DashboardModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(DashboardWidgetComponent);
    component = fixture.componentInstance;
    component.config = {
      id: '7ad74159-ce98-4d9a-a47a-1540a8a2ffe4',
      title: 'foo',
      type: 'value',
      value: {
        title: 'bar',
        value: '{{ xyz }} abc'
      }
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
