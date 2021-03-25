import { TestBed } from '@angular/core/testing';

import { AuthSessionService } from '~/app/shared/services/auth-session.service';

describe('AuthSessionService', () => {
  beforeEach(() => TestBed.configureTestingModule({}));

  it('should be created', () => {
    const service: AuthSessionService = TestBed.inject(AuthSessionService);
    expect(service).toBeTruthy();
  });
});
