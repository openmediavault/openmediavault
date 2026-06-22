import { TestBed } from '@angular/core/testing';
import { Router } from '@angular/router';

import { AuthService } from '~/app/shared/services/auth.service';
import { UnauthenticatedGuardService } from '~/app/shared/services/unauthenticated-guard.service';

describe('UnauthenticatedGuardService', () => {
  let service: UnauthenticatedGuardService;
  let authService: jest.Mocked<AuthService>;
  let router: jest.Mocked<Router>;

  beforeEach(() => {
    const authServiceMock = {
      authenticate: jest.fn(),
      logout: jest.fn(),
      isLoggedIn: jest.fn()
    } as unknown as jest.Mocked<AuthService>;
    const routerMock = {
      navigate: jest.fn()
    } as unknown as jest.Mocked<Router>;

    TestBed.configureTestingModule({
      providers: [
        UnauthenticatedGuardService,
        { provide: AuthService, useValue: authServiceMock },
        { provide: Router, useValue: routerMock }
      ]
    });

    service = TestBed.inject(UnauthenticatedGuardService);
    authService = TestBed.inject(AuthService) as jest.Mocked<AuthService>;
    router = TestBed.inject(Router) as jest.Mocked<Router>;
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });

  it('should allow access when user is not logged in', () => {
    authService.isLoggedIn.mockReturnValue(false);
    const result = service.canActivate(null, null);
    expect(result).toBe(true);
    expect(router.navigate).not.toHaveBeenCalled();
  });

  it('should redirect to dashboard when user is logged in', () => {
    authService.isLoggedIn.mockReturnValue(true);
    const result = service.canActivate(null, null);
    expect(result).toBe(false);
    expect(router.navigate).toHaveBeenCalledWith(['/dashboard']);
  });

  it('should allow child route access when user is not logged in', () => {
    authService.isLoggedIn.mockReturnValue(false);
    const result = service.canActivateChild(null, null);
    expect(result).toBe(true);
    expect(router.navigate).not.toHaveBeenCalled();
  });

  it('should redirect child route to dashboard when user is logged in', () => {
    authService.isLoggedIn.mockReturnValue(true);
    const result = service.canActivateChild(null, null);
    expect(result).toBe(false);
    expect(router.navigate).toHaveBeenCalledWith(['/dashboard']);
  });
});
