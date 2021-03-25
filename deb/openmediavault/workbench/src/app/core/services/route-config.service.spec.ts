import { HttpClientTestingModule } from '@angular/common/http/testing';
import { TestBed } from '@angular/core/testing';

import { RouteConfigService } from '~/app/core/services/route-config.service';

describe('RouteConfigService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [HttpClientTestingModule]
    })
  );

  it('should be created', () => {
    const service: RouteConfigService = TestBed.inject(RouteConfigService);
    expect(service).toBeTruthy();
  });
});
