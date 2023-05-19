import { TestBed } from '@angular/core/testing';

import { DashboardWidgetConfigService } from '~/app/core/services/dashboard-widget-config.service';
import { TestingModule } from '~/app/testing.module';

describe('DashboardWidgetConfigService', () => {
  let service: DashboardWidgetConfigService;

  beforeEach(() => {
    TestBed.configureTestingModule({
      imports: [TestingModule]
    });
    service = TestBed.inject(DashboardWidgetConfigService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
