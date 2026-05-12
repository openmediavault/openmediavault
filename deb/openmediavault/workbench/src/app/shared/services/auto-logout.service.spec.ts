import { TestBed } from '@angular/core/testing';

import { AutoLogoutService } from '~/app/shared/services/auto-logout.service';
import { TestingModule } from '~/app/testing.module';

describe('AutoLogoutService', () => {
  let service: AutoLogoutService;

  beforeEach(() => {
    TestBed.configureTestingModule({
      imports: [TestingModule]
    });
    service = TestBed.inject(AutoLogoutService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });

  it('should always return the same instance from the injector', () => {
    const secondInstance = TestBed.inject(AutoLogoutService);
    expect(secondInstance).toBe(service);
  });
});
