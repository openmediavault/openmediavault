import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { ToastrModule } from 'ngx-toastr';
import { of, throwError } from 'rxjs';

import { LoginPageComponent } from '~/app/core/pages/login-page/login-page.component';
import { PagesModule } from '~/app/core/pages/pages.module';
import { AuthService } from '~/app/shared/services/auth.service';
import { TestingModule } from '~/app/testing.module';

describe('LoginPageComponent', () => {
  let component: LoginPageComponent;
  let fixture: ComponentFixture<LoginPageComponent>;
  let mockAuthService: jest.Mocked<Pick<AuthService, 'login' | 'verifyTotp'>>;

  beforeEach(waitForAsync(() => {
    mockAuthService = {
      login: jest.fn(),
      verifyTotp: jest.fn()
    };

    TestBed.configureTestingModule({
      imports: [BrowserAnimationsModule, PagesModule, TestingModule, ToastrModule.forRoot()],
      providers: [{ provide: AuthService, useValue: mockAuthService }]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(LoginPageComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  it('should start at the credentials step', () => {
    expect(component.step).toBe('credentials');
  });

  describe('onLogin()', () => {
    it('should navigate to dashboard when MFA is not required', () => {
      mockAuthService.login.mockReturnValue(
        of({ authenticated: true, mfaRequired: false, username: 'admin', permissions: {} })
      );
      const routerSpy = jest.spyOn((component as any).router, 'navigate');

      component.onLogin({} as any, { username: 'admin', password: 'secret' });

      expect(component.step).toBe('credentials');
      expect(routerSpy).toHaveBeenCalledWith(['/dashboard']);
    });

    it('should advance to the TOTP step when MFA is required', () => {
      mockAuthService.login.mockReturnValue(
        of({
          authenticated: false,
          mfaRequired: true,
          username: 'admin',
          permissions: {},
          sessionid: 'pending'
        })
      );

      component.onLogin({} as any, { username: 'admin', password: 'secret' });

      expect(component.step).toBe('totp');
    });

    it('should remain on credentials step on login error', () => {
      mockAuthService.login.mockReturnValue(
        throwError(() => ({ error: { message: 'Incorrect username or password.' } }))
      );

      component.onLogin({} as any, { username: 'admin', password: 'wrong' });

      expect(component.step).toBe('credentials');
    });
  });

  describe('onVerifyTotp()', () => {
    beforeEach(() => {
      // Put component into the TOTP step first.
      component.step = 'totp';
    });

    it('should navigate to dashboard after successful TOTP verification', () => {
      mockAuthService.verifyTotp.mockReturnValue(
        of({ authenticated: true, mfaRequired: false, username: 'admin', permissions: {} })
      );
      const routerSpy = jest.spyOn((component as any).router, 'navigate');

      component.onVerifyTotp({} as any, { code: '123456' });

      expect(routerSpy).toHaveBeenCalledWith(['/dashboard']);
    });

    it('should stay on TOTP step when verification fails', () => {
      mockAuthService.verifyTotp.mockReturnValue(
        throwError(() => ({ error: { message: 'Invalid verification code.' } }))
      );

      component.onVerifyTotp({} as any, { code: '000000' });

      expect(component.step).toBe('totp');
    });
  });

  describe('onBackToCredentials()', () => {
    it('should return to the credentials step', () => {
      component.step = 'totp';

      component.onBackToCredentials({} as any, {});

      expect(component.step).toBe('credentials');
    });
  });
});
