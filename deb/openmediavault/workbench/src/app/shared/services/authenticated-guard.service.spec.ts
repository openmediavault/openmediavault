import { TestBed } from '@angular/core/testing';
import { Router, RouterStateSnapshot } from '@angular/router';

import { AuthService } from '~/app/shared/services/auth.service';
import { AuthenticatedGuardService } from '~/app/shared/services/authenticated-guard.service';

describe('AuthenticatedGuardService', () => {
  let service: AuthenticatedGuardService;
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
        AuthenticatedGuardService,
        { provide: AuthService, useValue: authServiceMock },
        { provide: Router, useValue: routerMock }
      ]
    });

    service = TestBed.inject(AuthenticatedGuardService);
    authService = TestBed.inject(AuthService) as jest.Mocked<AuthService>;
    router = TestBed.inject(Router) as jest.Mocked<Router>;
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });

  it('should allow access when user is logged in', () => {
    authService.isLoggedIn.mockReturnValue(true);
    const result = service.canActivate(null, { url: '/dashboard' } as RouterStateSnapshot);
    expect(result).toBe(true);
    expect(router.navigate).not.toHaveBeenCalled();
  });

  it('should redirect to login when user is not logged in', () => {
    authService.isLoggedIn.mockReturnValue(false);
    const result = service.canActivate(null, { url: '/dashboard' } as RouterStateSnapshot);
    expect(result).toBe(false);
    expect(router.navigate).toHaveBeenCalledWith(['/login'], {
      queryParams: { returnUrl: '/dashboard' }
    });
  });

  it('should redirect to login without returnUrl when accessing root path', () => {
    authService.isLoggedIn.mockReturnValue(false);
    const result = service.canActivate(null, { url: '/' } as RouterStateSnapshot);
    expect(result).toBe(false);
    expect(router.navigate).toHaveBeenCalledWith(['/login'], undefined);
  });

  it('should allow child route access when user is logged in', () => {
    authService.isLoggedIn.mockReturnValue(true);
    const result = service.canActivateChild(null, {
      url: '/system/settings'
    } as RouterStateSnapshot);
    expect(result).toBe(true);
    expect(router.navigate).not.toHaveBeenCalled();
  });

  it('should redirect child route to login when user is not logged in', () => {
    authService.isLoggedIn.mockReturnValue(false);
    const result = service.canActivateChild(null, {
      url: '/system/settings'
    } as RouterStateSnapshot);
    expect(result).toBe(false);
    expect(router.navigate).toHaveBeenCalledWith(['/login'], {
      queryParams: { returnUrl: '/system/settings' }
    });
  });
});
