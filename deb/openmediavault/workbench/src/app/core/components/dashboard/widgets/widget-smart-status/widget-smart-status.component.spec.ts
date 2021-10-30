/* eslint-disable max-len */
import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { WidgetSmartStatusComponent } from '~/app/core/components/dashboard/widgets/widget-smart-status/widget-smart-status.component';
import { TestingModule } from '~/app/testing.module';

describe('WidgetSmartStatusComponent', () => {
  let component: WidgetSmartStatusComponent;
  let fixture: ComponentFixture<WidgetSmartStatusComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [DashboardModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(WidgetSmartStatusComponent);
    component = fixture.componentInstance;
    component.config = {
      id: '2f06f7e5-b34f-47f7-a953-5eba2f386139',
      type: 'smart-status',
      title: 'bar'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
