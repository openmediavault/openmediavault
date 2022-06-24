/* eslint-disable max-len */
import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { DashboardWidgetFilesystemsStatusComponent } from '~/app/core/components/dashboard/dashboard-widget-filesystems-status/dashboard-widget-filesystems-status.component';
import { TestingModule } from '~/app/testing.module';

describe('DashboardWidgetFilesystemsStatusComponent', () => {
  let component: DashboardWidgetFilesystemsStatusComponent;
  let fixture: ComponentFixture<DashboardWidgetFilesystemsStatusComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [DashboardWidgetFilesystemsStatusComponent],
      imports: [DashboardModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(DashboardWidgetFilesystemsStatusComponent);
    component = fixture.componentInstance;
    component.config = {
      id: '2f06f7e5-b34f-47f7-a953-5eba2f386139',
      type: 'filesystems-status',
      title: 'baz'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
