import { TestBed } from '@angular/core/testing';

import { NavigationConfigService } from '~/app/core/services/navigation-config.service';
import { TestingModule } from '~/app/testing.module';

describe('NavigationConfigService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [TestingModule]
    })
  );

  it('should be created', () => {
    const service: NavigationConfigService = TestBed.inject(NavigationConfigService);
    expect(service).toBeTruthy();
  });
});
