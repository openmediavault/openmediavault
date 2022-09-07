/* eslint-disable max-len */
import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { WidgetFilesystemsStatusComponent } from '~/app/core/components/dashboard/widgets/widget-filesystems-status/widget-filesystems-status.component';
import { TestingModule } from '~/app/testing.module';

describe('WidgetFilesystemsStatusComponent', () => {
  let component: WidgetFilesystemsStatusComponent;
  let fixture: ComponentFixture<WidgetFilesystemsStatusComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [WidgetFilesystemsStatusComponent],
      imports: [DashboardModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(WidgetFilesystemsStatusComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
