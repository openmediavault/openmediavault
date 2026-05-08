// Mock the qrcode module so QRCode.toDataURL() resolves immediately in jsdom
// without requiring a real canvas implementation.
jest.mock('qrcode', () => ({
  toDataURL: jest.fn().mockResolvedValue('data:image/png;base64,AAAA')
}));

import { NO_ERRORS_SCHEMA } from '@angular/core';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { ReactiveFormsModule } from '@angular/forms';
import { of, throwError } from 'rxjs';
import { ToastrModule } from 'ngx-toastr';

import { UserMfaFormPageComponent } from '~/app/pages/usermgmt/users/user-mfa-form-page.component';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';
import { TestingModule } from '~/app/testing.module';

describe('UserMfaFormPageComponent', () => {
  let component: UserMfaFormPageComponent;
  let fixture: ComponentFixture<UserMfaFormPageComponent>;
  let mockRpcService: jest.Mocked<Pick<RpcService, 'request'>>;
  let mockNotificationService: jest.Mocked<Pick<NotificationService, 'show'>>;

  beforeEach(waitForAsync(() => {
    mockRpcService = { request: jest.fn() };
    mockNotificationService = { show: jest.fn() };

    // Default: return MFA disabled so the component finishes loading immediately.
    mockRpcService.request.mockReturnValue(of({ enabled: false }));

    TestBed.configureTestingModule({
      declarations: [UserMfaFormPageComponent],
      imports: [
        ReactiveFormsModule,
        TestingModule,
        ToastrModule.forRoot()
      ],
      providers: [
        { provide: RpcService, useValue: mockRpcService },
        { provide: NotificationService, useValue: mockNotificationService }
      ],
      // NO_ERRORS_SCHEMA suppresses "unknown element" errors for Material
      // components that are not the focus of these unit tests. The Material
      // elements are already covered by the UsermgmtModule integration.
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
    it('should transition to "setup" state and store the provisional secret', async () => {
      mockRpcService.request.mockReturnValue(
        of({ secret: 'JBSWY3DPEHPK3PXP', uri: 'otpauth://totp/OMV:admin?secret=JBSWY3DPEHPK3PXP' })
      );
      component.state = 'disabled';

      component.onStartSetup();

      // Wait for the QR code Promise to resolve before asserting.
      await fixture.whenStable();

      expect(component.state).toBe('setup');
      expect(component.provisionalSecret).toBe('JBSWY3DPEHPK3PXP');
      expect(component.qrCodeDataUrl).toMatch(/^data:image\/png;base64,/);
    });
  });

  describe('onConfirmSetup()', () => {
    beforeEach(() => {
      component.state = 'setup';
      component.provisionalSecret = 'JBSWY3DPEHPK3PXP';
    });

    it('should enable MFA and transition to "enabled" when code is valid', () => {
      mockRpcService.request.mockReturnValue(of(null));
      component.setupCodeControl.setValue('123456');

      component.onConfirmSetup();

      expect(mockRpcService.request).toHaveBeenCalledWith(
        'UserMgmt',
        'enableTotpByContext',
        { code: '123456', secret: 'JBSWY3DPEHPK3PXP' }
      );
      expect(component.state).toBe('enabled');
      expect(component.provisionalSecret).toBe('');
    });

    it('should not submit when the code input is invalid', () => {
      component.setupCodeControl.setValue('12'); // too short

      component.onConfirmSetup();

      expect(mockRpcService.request).not.toHaveBeenCalledWith(
        'UserMgmt', 'enableTotpByContext', expect.anything()
      );
    });
  });

  describe('onCancelSetup()', () => {
    it('should return to "disabled" state and clear all setup data', () => {
      component.state = 'setup';
      component.provisionalSecret = 'JBSWY3DPEHPK3PXP';
      component.qrCodeDataUrl = 'data:image/png;base64,...';
      component.setupCodeControl.setValue('123456');

      component.onCancelSetup();

      expect(component.state).toBe('disabled');
      expect(component.provisionalSecret).toBe('');
      expect(component.qrCodeDataUrl).toBe('');
      expect(component.setupCodeControl.value).toBeNull();
    });
  });

  describe('onDisableMfa()', () => {
    beforeEach(() => {
      component.state = 'enabled';
    });

    it('should disable MFA and transition to "disabled" when code is valid', () => {
      mockRpcService.request.mockReturnValue(of(null));
      component.disableCodeControl.setValue('654321');

      component.onDisableMfa();

      expect(mockRpcService.request).toHaveBeenCalledWith(
        'UserMgmt',
        'disableTotpByContext',
        { code: '654321' }
      );
      expect(component.state).toBe('disabled');
    });

    it('should not submit when the disable code input is invalid', () => {
      component.disableCodeControl.setValue(''); // empty

      component.onDisableMfa();

      expect(mockRpcService.request).not.toHaveBeenCalledWith(
        'UserMgmt', 'disableTotpByContext', expect.anything()
      );
    });
  });
});
