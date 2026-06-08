import { TestBed } from '@angular/core/testing';
import { of, throwError } from 'rxjs';

import { AuthService, SessionData } from '~/app/shared/services/auth.service';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { PrefersColorSchemeService } from '~/app/shared/services/prefers-color-scheme.service';
import { RpcService } from '~/app/shared/services/rpc.service';
import { TestingModule } from '~/app/testing.module';

describe('AuthService', () => {
  let service: AuthService;
  let mockRpcService: jest.Mocked<Pick<RpcService, 'request'>>;
  let mockAuthSessionService: jest.Mocked<
    Pick<AuthSessionService, 'set' | 'revoke' | 'getUsername'>
  >;

  const sessionDataNoMfa: SessionData = {
    authenticated: true,
    mfaRequired: false,
    username: 'admin',
    permissions: { role: 'admin' },
    sessionid: 'sess-1'
  };

  const sessionDataMfaRequired: SessionData = {
    authenticated: false,
    mfaRequired: true,
    username: 'admin',
    permissions: { role: 'admin' },
    sessionid: 'sess-pending'
  };

  const sessionDataMfaComplete: SessionData = {
    authenticated: true,
    mfaRequired: false,
    username: 'admin',
    permissions: { role: 'admin' },
    sessionid: 'sess-1'
  };

  beforeEach(() => {
    mockRpcService = { request: jest.fn() };
    mockAuthSessionService = {
      set: jest.fn(),
      revoke: jest.fn(),
      // getUsername is called by PrefersColorSchemeService on construction.
      getUsername: jest.fn().mockReturnValue(null)
    };

    TestBed.configureTestingModule({
      imports: [TestingModule],
      providers: [
        { provide: RpcService, useValue: mockRpcService },
        { provide: AuthSessionService, useValue: mockAuthSessionService },
        // PrefersColorSchemeService depends on AuthSessionService via
        // UserLocalStorageService. Providing our mock for AuthSessionService
        // is enough — no need to stub PrefersColorSchemeService itself.
        PrefersColorSchemeService
      ]
    });
    service = TestBed.inject(AuthService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });

  describe('login()', () => {
    it('should store session when authentication succeeds without MFA', (done) => {
      mockRpcService.request.mockReturnValue(of(sessionDataNoMfa));

      service.login('admin', 'secret').subscribe((res) => {
        expect(res.authenticated).toBe(true);
        expect(res.mfaRequired).toBe(false);
        expect(mockAuthSessionService.set).toHaveBeenCalledWith('admin', { role: 'admin' });
        done();
      });
    });

    it('should NOT store session when MFA is required', (done) => {
      mockRpcService.request.mockReturnValue(of(sessionDataMfaRequired));

      service.login('admin', 'secret').subscribe((res) => {
        expect(res.authenticated).toBe(false);
        expect(res.mfaRequired).toBe(true);
        // Session must not be considered authenticated until TOTP is verified.
        expect(mockAuthSessionService.set).not.toHaveBeenCalled();
        done();
      });
    });

    it('should call the correct RPC method with credentials', () => {
      mockRpcService.request.mockReturnValue(of(sessionDataNoMfa));

      service.login('admin', 'password').subscribe();

      expect(mockRpcService.request).toHaveBeenCalledWith('Session', 'login', {
        username: 'admin',
        password: 'password'
      });
    });
  });

  describe('verifyTotp()', () => {
    it('should store session after successful TOTP verification', (done) => {
      mockRpcService.request.mockReturnValue(of(sessionDataMfaComplete));

      service.verifyTotp('123456').subscribe((res) => {
        expect(res.authenticated).toBe(true);
        expect(mockAuthSessionService.set).toHaveBeenCalledWith('admin', { role: 'admin' });
        done();
      });
    });

    it('should call the correct RPC method with the TOTP code', () => {
      mockRpcService.request.mockReturnValue(of(sessionDataMfaComplete));

      service.verifyTotp('654321').subscribe();

      expect(mockRpcService.request).toHaveBeenCalledWith('Session', 'verifyTotp', {
        code: '654321'
      });
    });

    it('should propagate RPC errors on invalid TOTP code', (done) => {
      mockRpcService.request.mockReturnValue(
        throwError(() => ({ error: { message: 'Invalid verification code.' } }))
      );

      service.verifyTotp('000000').subscribe({
        error: (err) => {
          expect(mockAuthSessionService.set).not.toHaveBeenCalled();
          expect(err).toBeTruthy();
          done();
        }
      });
    });
  });
});
