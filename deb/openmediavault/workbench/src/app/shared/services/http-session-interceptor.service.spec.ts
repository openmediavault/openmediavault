import { HttpRequest, HttpResponse } from '@angular/common/http';
import { TestBed } from '@angular/core/testing';
import { of } from 'rxjs';

import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { HttpSessionInterceptorService } from '~/app/shared/services/http-session-interceptor.service';

describe('HttpSessionInterceptorService', () => {
  let httpSessionInterceptorService: HttpSessionInterceptorService;
  let authSessionService: AuthSessionService;
  let nextHandler: any;

  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [HttpSessionInterceptorService, AuthSessionService]
    });

    httpSessionInterceptorService = TestBed.inject(HttpSessionInterceptorService);
    authSessionService = TestBed.inject(AuthSessionService);

    nextHandler = {
      handle: jest.fn()
    };
  });

  it('should create', () => {
    expect(httpSessionInterceptorService).toBeTruthy();
  });

  describe('Session.authenticate', () => {
    it('should set session when status is authenticated', (done) => {
      const request = new HttpRequest('POST', 'rpc.php', {
        service: 'Session',
        method: 'authenticate',
        params: { username: 'admin', password: 'secret' }
      });

      const response = new HttpResponse({
        body: {
          response: {
            status: 'authenticated',
            username: 'admin',
            permissions: { role: ['admin'] },
            sessionid: 'abc123'
          }
        }
      });

      nextHandler.handle.mockReturnValue(of(response));

      const setSpy = jest.spyOn(authSessionService, 'set');

      httpSessionInterceptorService.intercept(request, nextHandler).subscribe(() => {
        expect(setSpy).toHaveBeenCalledWith('admin', { role: ['admin'] });
        done();
      });
    });

    it('should NOT set session when status is challengeRequired', (done) => {
      const request = new HttpRequest('POST', 'rpc.php', {
        service: 'Session',
        method: 'authenticate',
        params: { username: 'admin', password: 'secret' }
      });

      const response = new HttpResponse({
        body: {
          response: {
            status: 'challengeRequired',
            username: 'admin',
            challenge: { kind: 'totp', redirecturl: '/2fa-totp' }
          }
        }
      });

      nextHandler.handle.mockReturnValue(of(response));

      const setSpy = jest.spyOn(authSessionService, 'set');

      httpSessionInterceptorService.intercept(request, nextHandler).subscribe(() => {
        expect(setSpy).not.toHaveBeenCalled();
        done();
      });
    });

    it('should NOT set session when username is missing', (done) => {
      const request = new HttpRequest('POST', 'rpc.php', {
        service: 'Session',
        method: 'authenticate'
      });

      const response = new HttpResponse({
        body: {
          response: {
            status: 'authenticated',
            permissions: { role: ['admin'] }
          }
        }
      });

      nextHandler.handle.mockReturnValue(of(response));

      const setSpy = jest.spyOn(authSessionService, 'set');

      httpSessionInterceptorService.intercept(request, nextHandler).subscribe(() => {
        expect(setSpy).not.toHaveBeenCalled();
        done();
      });
    });

    it('should NOT set session when permissions are missing', (done) => {
      const request = new HttpRequest('POST', 'rpc.php', {
        service: 'Session',
        method: 'authenticate'
      });

      const response = new HttpResponse({
        body: {
          response: {
            status: 'authenticated',
            username: 'admin'
          }
        }
      });

      nextHandler.handle.mockReturnValue(of(response));

      const setSpy = jest.spyOn(authSessionService, 'set');

      httpSessionInterceptorService.intercept(request, nextHandler).subscribe(() => {
        expect(setSpy).not.toHaveBeenCalled();
        done();
      });
    });
  });

  describe('Session.verify', () => {
    it('should set session on successful verify', (done) => {
      const request = new HttpRequest('POST', 'rpc.php', {
        service: 'Session',
        method: 'verify',
        params: { code: '123456' }
      });

      const response = new HttpResponse({
        body: {
          response: {
            authenticated: true,
            username: 'admin',
            permissions: { role: ['admin'] },
            sessionid: 'abc123'
          }
        }
      });

      nextHandler.handle.mockReturnValue(of(response));

      const setSpy = jest.spyOn(authSessionService, 'set');

      httpSessionInterceptorService.intercept(request, nextHandler).subscribe(() => {
        expect(setSpy).toHaveBeenCalledWith('admin', { role: ['admin'] });
        done();
      });
    });

    it('should NOT set session when username is missing', (done) => {
      const request = new HttpRequest('POST', 'rpc.php', {
        service: 'Session',
        method: 'verify'
      });

      const response = new HttpResponse({
        body: {
          response: {
            authenticated: true,
            permissions: { role: ['admin'] }
          }
        }
      });

      nextHandler.handle.mockReturnValue(of(response));

      const setSpy = jest.spyOn(authSessionService, 'set');

      httpSessionInterceptorService.intercept(request, nextHandler).subscribe(() => {
        expect(setSpy).not.toHaveBeenCalled();
        done();
      });
    });

    it('should NOT set session when permissions are missing', (done) => {
      const request = new HttpRequest('POST', 'rpc.php', {
        service: 'Session',
        method: 'verify'
      });

      const response = new HttpResponse({
        body: {
          response: {
            authenticated: true,
            username: 'admin'
          }
        }
      });

      nextHandler.handle.mockReturnValue(of(response));

      const setSpy = jest.spyOn(authSessionService, 'set');

      httpSessionInterceptorService.intercept(request, nextHandler).subscribe(() => {
        expect(setSpy).not.toHaveBeenCalled();
        done();
      });
    });
  });

  describe('Session.logout', () => {
    it('should revoke session on logout', (done) => {
      const request = new HttpRequest('POST', 'rpc.php', {
        service: 'Session',
        method: 'logout'
      });

      const response = new HttpResponse({
        body: {
          response: null
        }
      });

      nextHandler.handle.mockReturnValue(of(response));

      const revokeSpy = jest.spyOn(authSessionService, 'revoke');
      const setSpy = jest.spyOn(authSessionService, 'set');

      httpSessionInterceptorService.intercept(request, nextHandler).subscribe(() => {
        expect(revokeSpy).toHaveBeenCalled();
        expect(setSpy).not.toHaveBeenCalled();
        done();
      });
    });

    it('should revoke session even if response is empty', (done) => {
      const request = new HttpRequest('POST', 'rpc.php', {
        service: 'Session',
        method: 'logout'
      });

      const response = new HttpResponse({
        body: {}
      });

      nextHandler.handle.mockReturnValue(of(response));

      const revokeSpy = jest.spyOn(authSessionService, 'revoke');

      httpSessionInterceptorService.intercept(request, nextHandler).subscribe(() => {
        expect(revokeSpy).toHaveBeenCalled();
        done();
      });
    });
  });

  describe('Other requests', () => {
    it('should ignore non-RPC requests', (done) => {
      const request = new HttpRequest('GET', '/api/data');
      const response = new HttpResponse({ body: { data: [] } });

      nextHandler.handle.mockReturnValue(of(response));

      const setSpy = jest.spyOn(authSessionService, 'set');
      const revokeSpy = jest.spyOn(authSessionService, 'revoke');

      httpSessionInterceptorService.intercept(request, nextHandler).subscribe(() => {
        expect(setSpy).not.toHaveBeenCalled();
        expect(revokeSpy).not.toHaveBeenCalled();
        done();
      });
    });

    it('should handle rpc.php with query parameters', (done) => {
      const request = new HttpRequest('POST', 'rpc.php?foo=bar', {
        service: 'Session',
        method: 'logout'
      });

      const response = new HttpResponse({ body: { response: null } });

      nextHandler.handle.mockReturnValue(of(response));

      const revokeSpy = jest.spyOn(authSessionService, 'revoke');

      httpSessionInterceptorService.intercept(request, nextHandler).subscribe(() => {
        expect(revokeSpy).toHaveBeenCalled();
        done();
      });
    });

    it('should ignore requests with non-string service or method', (done) => {
      const request = new HttpRequest('POST', 'rpc.php', {
        service: 123,
        method: null
      });

      const response = new HttpResponse({ body: { response: null } });

      nextHandler.handle.mockReturnValue(of(response));

      const setSpy = jest.spyOn(authSessionService, 'set');
      const revokeSpy = jest.spyOn(authSessionService, 'revoke');

      httpSessionInterceptorService.intercept(request, nextHandler).subscribe(() => {
        expect(setSpy).not.toHaveBeenCalled();
        expect(revokeSpy).not.toHaveBeenCalled();
        done();
      });
    });

    it('should ignore non-Session service RPCs', (done) => {
      const request = new HttpRequest('POST', 'rpc.php', {
        service: 'User',
        method: 'getList'
      });

      const response = new HttpResponse({
        body: {
          response: { data: [] }
        }
      });

      nextHandler.handle.mockReturnValue(of(response));

      const setSpy = jest.spyOn(authSessionService, 'set');
      const revokeSpy = jest.spyOn(authSessionService, 'revoke');

      httpSessionInterceptorService.intercept(request, nextHandler).subscribe(() => {
        expect(setSpy).not.toHaveBeenCalled();
        expect(revokeSpy).not.toHaveBeenCalled();
        done();
      });
    });

    it('should ignore other Session methods', (done) => {
      const request = new HttpRequest('POST', 'rpc.php', {
        service: 'Session',
        method: 'getInfo'
      });

      const response = new HttpResponse({
        body: {
          response: { authenticated: true }
        }
      });

      nextHandler.handle.mockReturnValue(of(response));

      const setSpy = jest.spyOn(authSessionService, 'set');
      const revokeSpy = jest.spyOn(authSessionService, 'revoke');

      httpSessionInterceptorService.intercept(request, nextHandler).subscribe(() => {
        expect(setSpy).not.toHaveBeenCalled();
        expect(revokeSpy).not.toHaveBeenCalled();
        done();
      });
    });
  });
});
