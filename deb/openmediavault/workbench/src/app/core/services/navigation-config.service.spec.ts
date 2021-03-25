import { HttpClientTestingModule } from '@angular/common/http/testing';
import { TestBed } from '@angular/core/testing';

import { NavigationConfigService } from '~/app/core/services/navigation-config.service';

describe('NavigationConfigService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [HttpClientTestingModule]
    })
  );

  it('should be created', () => {
    const service: NavigationConfigService = TestBed.inject(NavigationConfigService);
    expect(service).toBeTruthy();
  });
});
