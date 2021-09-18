import { TestBed } from '@angular/core/testing';

import { AuthGuardService } from '~/app/shared/services/auth-guard.service';
import { TestingModule } from '~/app/testing.module';

describe('AuthGuardService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [TestingModule]
    })
  );

  it('should be created', () => {
    const service: AuthGuardService = TestBed.inject(AuthGuardService);
    expect(service).toBeTruthy();
  });
});
