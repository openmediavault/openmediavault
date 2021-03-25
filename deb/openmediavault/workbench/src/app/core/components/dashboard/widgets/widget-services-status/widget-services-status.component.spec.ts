/* eslint-disable max-len */
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { WidgetServicesStatusComponent } from '~/app/core/components/dashboard/widgets/widget-services-status/widget-services-status.component';

describe('WidgetServicesStatusComponent', () => {
  let component: WidgetServicesStatusComponent;
  let fixture: ComponentFixture<WidgetServicesStatusComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [DashboardModule, HttpClientTestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(WidgetServicesStatusComponent);
    component = fixture.componentInstance;
    component.config = {
      id: '94bf613a-8bce-11eb-9be2-cf920ddc0976',
      type: 'services-status',
      title: 'foo'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
