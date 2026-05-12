import { TestBed } from '@angular/core/testing';

import { LogConfigService } from '~/app/core/services/log-config.service';
import { TestingModule } from '~/app/testing.module';

describe('LogConfigService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [TestingModule]
    })
  );

  it('should be created', () => {
    const service: LogConfigService = TestBed.inject(LogConfigService);
    expect(service).toBeTruthy();
  });
});
