/* eslint-disable max-len */
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { TranslateModule } from '@ngx-translate/core';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { WidgetSystemInformationComponent } from '~/app/core/components/dashboard/widgets/widget-system-information/widget-system-information.component';

describe('WidgetSystemInformationComponent', () => {
  let component: WidgetSystemInformationComponent;
  let fixture: ComponentFixture<WidgetSystemInformationComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [
          DashboardModule,
          HttpClientTestingModule,
          RouterTestingModule,
          TranslateModule.forRoot()
        ]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(WidgetSystemInformationComponent);
    component = fixture.componentInstance;
    component.config = {
      id: '3b7d97dc-f5d3-4a5b-9300-830a0946e18a',
      type: 'system-information',
      title: 'foo'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
