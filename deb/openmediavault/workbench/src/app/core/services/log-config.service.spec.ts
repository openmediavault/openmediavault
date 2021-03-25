import { HttpClientTestingModule } from '@angular/common/http/testing';
import { TestBed } from '@angular/core/testing';

import { LogConfigService } from '~/app/core/services/log-config.service';

describe('LogConfigService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [HttpClientTestingModule]
    })
  );

  it('should be created', () => {
    const service: LogConfigService = TestBed.inject(LogConfigService);
    expect(service).toBeTruthy();
  });
});
