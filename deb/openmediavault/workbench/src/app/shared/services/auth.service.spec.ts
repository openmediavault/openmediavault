import { TestBed } from '@angular/core/testing';

import { AuthService } from '~/app/shared/services/auth.service';
import { TestingModule } from '~/app/testing.module';

describe('AuthService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [TestingModule]
    })
  );

  it('should be created', () => {
    const service: AuthService = TestBed.inject(AuthService);
    expect(service).toBeTruthy();
  });
});
