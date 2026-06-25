import { TestBed } from '@angular/core/testing';
import { of, Subject } from 'rxjs';

import { AuthService } from '~/app/shared/services/auth.service';
import { AutoLogoutService } from '~/app/shared/services/auto-logout.service';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcRequestWithResponse, RpcService } from '~/app/shared/services/rpc.service';
import { UserLocalStorageService } from '~/app/shared/services/user-local-storage.service';
import { TestingModule } from '~/app/testing.module';

describe('AutoLogoutService', () => {
  let service: AutoLogoutService;
  let authService: { logout: jest.Mock };
  let notificationService: { show: jest.Mock };
  let userLocalStorageService: {
    buildKey: jest.Mock;
    set: jest.Mock;
    remove: jest.Mock;
  };
  let requestSource: Subject<RpcRequestWithResponse>;
  let rpcService: {
    request$: Subject<RpcRequestWithResponse>;
    request: jest.Mock;
  };

  beforeEach(() => {
    authService = {
      logout: jest.fn(() => of(undefined))
    };
    notificationService = {
      show: jest.fn()
    };
    userLocalStorageService = {
      buildKey: jest.fn(() => 'admin@lastActivity'),
      set: jest.fn(),
      remove: jest.fn()
    };
    requestSource = new Subject<RpcRequestWithResponse>();
    rpcService = {
      request$: requestSource,
      request: jest.fn(() => of({ timeout: 5 }))
    };

    TestBed.configureTestingModule({
      imports: [TestingModule],
      providers: [
        { provide: AuthService, useValue: authService },
        { provide: NotificationService, useValue: notificationService },
        { provide: RpcService, useValue: rpcService },
        { provide: UserLocalStorageService, useValue: userLocalStorageService }
      ]
    });
    service = TestBed.inject(AutoLogoutService);
    jest.useFakeTimers();
  });

  afterEach(() => {
    service.stop();
    requestSource.complete();
    jest.useRealTimers();
    jest.clearAllMocks();
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });

  it('should always return the same instance from the injector', () => {
    const secondInstance = TestBed.inject(AutoLogoutService);
    expect(secondInstance).toBe(service);
  });

  it('should capture keyboard activity even if propagation is stopped', () => {
    (service as any).timeout = 60000;
    service.start();

    const input = document.createElement('input');
    input.addEventListener('keydown', (event) => event.stopPropagation());
    document.body.appendChild(input);

    input.dispatchEvent(new KeyboardEvent('keydown', { key: 'a', bubbles: true }));

    expect(userLocalStorageService.set).toHaveBeenCalledWith(
      'lastActivity',
      expect.any(String),
      true
    );
    document.body.removeChild(input);
  });

  it('should reset the timer on activity', () => {
    (service as any).timeout = 60000;
    service.start();

    jest.advanceTimersByTime(55000);
    expect(authService.logout).not.toHaveBeenCalled();

    window.dispatchEvent(new KeyboardEvent('keydown', { key: 'a', bubbles: true }));

    jest.advanceTimersByTime(59000);
    expect(authService.logout).not.toHaveBeenCalled();

    jest.advanceTimersByTime(1000);
    expect(authService.logout).toHaveBeenCalledTimes(1);
  });

  it('should sanitize invalid timeout values loaded from RPC', () => {
    rpcService.request.mockReturnValue(of({ timeout: 'invalid' }));
    service.load().subscribe();

    expect((service as any).timeout).toBe(0);
  });

  it('should sanitize invalid timeout values from WebGui settings updates', () => {
    requestSource.next({
      service: 'WebGui',
      method: 'setSettings',
      response: {
        timeout: undefined
      }
    });

    expect((service as any).timeout).toBe(0);
  });
});
