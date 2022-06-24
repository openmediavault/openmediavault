/* eslint-disable max-len */
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { ToastrModule } from 'ngx-toastr';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { DashboardWidgetDatatableComponent } from '~/app/core/components/dashboard/dashboard-widget-datatable/dashboard-widget-datatable.component';
import { TestingModule } from '~/app/testing.module';

describe('DashboardWidgetDatatableComponent', () => {
  let component: DashboardWidgetDatatableComponent;
  let fixture: ComponentFixture<DashboardWidgetDatatableComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      declarations: [DashboardWidgetDatatableComponent],
      imports: [DashboardModule, TestingModule, ToastrModule.forRoot()]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DashboardWidgetDatatableComponent);
    component = fixture.componentInstance;
    component.config = {
      id: '288d9453-72f0-4aca-9d8f-e4582b6b07b4',
      type: 'datatable',
      title: 'xyz'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
