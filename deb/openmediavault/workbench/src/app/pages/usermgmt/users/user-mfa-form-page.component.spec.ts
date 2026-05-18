import { NO_ERRORS_SCHEMA } from '@angular/core';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { Router } from '@angular/router';
import { of, throwError } from 'rxjs';
import { ToastrModule } from 'ngx-toastr';

import { UserMfaFormPageComponent } from '~/app/pages/usermgmt/users/user-mfa-form-page.component';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';
import { BlockUiService } from '~/app/shared/services/block-ui.service';
import { TestingModule } from '~/app/testing.module';

describe('UserMfaFormPageComponent', () => {
  let component: UserMfaFormPageComponent;
  let fixture: ComponentFixture<UserMfaFormPageComponent>;
  let mockRpcService: jest.Mocked<Pick<RpcService, 'request'>>;
  let mockNotificationService: jest.Mocked<Pick<NotificationService, 'show'>>;
  let mockBlockUiService: jest.Mocked<Pick<BlockUiService, 'start' | 'stop'>>;
  let mockRouter: jest.Mocked<Pick<Router, 'navigate'>>;

  beforeEach(waitForAsync(() => {
    mockRpcService = { request: jest.fn() };
    mockNotificationService = { show: jest.fn() };
    mockBlockUiService = { start: jest.fn(), stop: jest.fn() };
    mockRouter = { navigate: jest.fn() };

    // Default: return MFA disabled so the component finishes loading immediately.
    mockRpcService.request.mockReturnValue(of({ enabled: false }));

    TestBed.configureTestingModule({
      declarations: [UserMfaFormPageComponent],
      imports: [TestingModule, ToastrModule.forRoot()],
      providers: [
        { provide: RpcService, useValue: mockRpcService },
        { provide: NotificationService, useValue: mockNotificationService },
        { provide: BlockUiService, useValue: mockBlockUiService },
        { provide: Router, useValue: mockRouter }
      ],
      // NO_ERRORS_SCHEMA suppresses "unknown element" errors for child
      // components (omv-intuition-form-page) not under test here.
      schemas: [NO_ERRORS_SCHEMA]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(UserMfaFormPageComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  describe('loadStatus()', () => {
    it('should set state to "disabled" when MFA is off', () => {
      mockRpcService.request.mockReturnValue(of({ enabled: false }));
      component.loadStatus();
      expect(component.state).toBe('disabled');
    });

    it('should set state to "enabled" when MFA is on', () => {
      mockRpcService.request.mockReturnValue(of({ enabled: true }));
      component.loadStatus();
      expect(component.state).toBe('enabled');
    });

    it('should fall back to "disabled" on RPC error', () => {
      mockRpcService.request.mockReturnValue(
        throwError(() => new Error('Network error'))
      );
      component.loadStatus();
      expect(component.state).toBe('disabled');
    });
  });

  describe('onStartSetup()', () => {
    it('should transition to "setup" state and populate setupConfig fields', () => {
      const qrDataUrl = 'data:image/svg+xml;base64,AAAA';
      mockRpcService.request.mockReturnValue(
        of({ secret: 'JBSWY3DPEHPK3PXP', uri: 'otpauth://totp/OMV:admin?secret=JBSWY3DPEHPK3PXP', qrDataUrl })
      );
      component.state = 'disabled';

      component.onStartSetup(null, {});

      expect(component.state).toBe('setup');

      const qrField = component.setupConfig.fields.find((f) => f.name === 'qrcode');
      expect(qrField?.value).toBe(qrDataUrl);

      const secretField = component.setupConfig.fields.find((f) => f.name === 'secret');
      expect(secretField?.value).toBe('JBSWY3DPEHPK3PXP');
    });
  });

  describe('onConfirmSetup()', () => {
    it('should enable MFA and transition to "enabled"', () => {
      mockRpcService.request.mockReturnValue(of(null));
      component.state = 'setup';
      (component as any).provisionalSecret = 'JBSWY3DPEHPK3PXP';

      component.onConfirmSetup(null, { code: '123456' });

      expect(mockRpcService.request).toHaveBeenCalledWith(
        'UserMgmt',
        'enableTotpByContext',
        { code: '123456', secret: 'JBSWY3DPEHPK3PXP' }
      );
      expect(component.state).toBe('enabled');
    });
  });

  describe('onCancelSetup()', () => {
    it('should return to "disabled" state and clear the provisional secret', () => {
      component.state = 'setup';
      (component as any).provisionalSecret = 'JBSWY3DPEHPK3PXP';

      component.onCancelSetup(null, {});

      expect(component.state).toBe('disabled');
      expect((component as any).provisionalSecret).toBe('');
    });
  });

  describe('onDisableMfa()', () => {
    it('should disable MFA and transition to "disabled"', () => {
      mockRpcService.request.mockReturnValue(of(null));
      component.state = 'enabled';

      component.onDisableMfa(null, { code: '654321' });

      expect(mockRpcService.request).toHaveBeenCalledWith(
        'UserMgmt',
        'disableTotpByContext',
        { code: '654321' }
      );
      expect(component.state).toBe('disabled');
    });
  });

  describe('onCancel()', () => {
    it('should navigate to the root route', () => {
      component.onCancel();
      expect(mockRouter.navigate).toHaveBeenCalledWith(['/']);
    });
  });
});
