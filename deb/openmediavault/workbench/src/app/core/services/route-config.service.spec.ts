import { TestBed } from '@angular/core/testing';

import { RouteConfigService } from '~/app/core/services/route-config.service';
import { TestingModule } from '~/app/testing.module';

describe('RouteConfigService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [TestingModule]
    })
  );

  it('should be created', () => {
    const service: RouteConfigService = TestBed.inject(RouteConfigService);
    expect(service).toBeTruthy();
  });
});
