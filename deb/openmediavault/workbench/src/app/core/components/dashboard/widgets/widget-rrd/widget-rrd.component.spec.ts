import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { TranslateModule } from '@ngx-translate/core';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { WidgetRrdComponent } from '~/app/core/components/dashboard/widgets/widget-rrd/widget-rrd.component';
import { TestingModule } from '~/app/testing.module';

describe('WidgetRrdComponent', () => {
  let component: WidgetRrdComponent;
  let fixture: ComponentFixture<WidgetRrdComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [DashboardModule, TestingModule, TranslateModule.forRoot()]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(WidgetRrdComponent);
    component = fixture.componentInstance;
    component.config = {
      id: 'b7e2e07e-4b61-4b39-86f6-df217edd91c7',
      type: 'rrd',
      title: 'abc'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
