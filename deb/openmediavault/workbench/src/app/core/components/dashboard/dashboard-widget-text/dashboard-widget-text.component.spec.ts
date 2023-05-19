/* eslint-disable max-len */
import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { DashboardWidgetTextComponent } from '~/app/core/components/dashboard/dashboard-widget-text/dashboard-widget-text.component';
import { TestingModule } from '~/app/testing.module';

describe('DashboardWidgetTextComponent', () => {
  let component: DashboardWidgetTextComponent;
  let fixture: ComponentFixture<DashboardWidgetTextComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [DashboardWidgetTextComponent],
      imports: [DashboardModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(DashboardWidgetTextComponent);
    component = fixture.componentInstance;
    component.config = {
      id: 'fd896a2c-5287-49d7-b773-e25c62071540',
      type: 'text',
      title: 'foo'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
