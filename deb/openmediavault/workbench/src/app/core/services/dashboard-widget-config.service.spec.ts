import { Platform } from '@angular/cdk/platform';
import { TestBed } from '@angular/core/testing';
import { of, throwError } from 'rxjs';

import { DashboardUserConfig } from '~/app/core/components/dashboard/models/dashboard-user-config.model';
import { DashboardWidgetConfigService } from '~/app/core/services/dashboard-widget-config.service';
import { RpcService } from '~/app/shared/services/rpc.service';
import { UserLocalStorageService } from '~/app/shared/services/user-local-storage.service';
import { TestingModule } from '~/app/testing.module';

describe('DashboardWidgetConfigService', () => {
  let service: DashboardWidgetConfigService;
  let rpcService: RpcService;
  let userLocalStorageService: UserLocalStorageService;
  let platform: Platform;

  beforeEach(() => {
    TestBed.configureTestingModule({
      imports: [TestingModule]
    });
    service = TestBed.inject(DashboardWidgetConfigService);
    rpcService = TestBed.inject(RpcService);
    userLocalStorageService = TestBed.inject(UserLocalStorageService);
    platform = TestBed.inject(Platform);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });

  it('should returns settings from service with desktop', (done) => {
    const stubValue: DashboardUserConfig = { widgets: [{ id: '12345' }] };
    jest.spyOn(rpcService, 'request').mockReturnValueOnce(of(stubValue));
    jest.spyOn(userLocalStorageService, 'set');

    service.loadUserWidgetConfig().subscribe((value) => {
      expect(rpcService.request).toHaveBeenCalledTimes(1);
      expect(rpcService.request).toHaveBeenCalledWith('Dashboard', 'getDashboardSetting', {
        deviceType: 'desktop'
      });
      expect(userLocalStorageService.set).toHaveBeenCalledTimes(1);
      expect(userLocalStorageService.set).toHaveBeenCalledWith(
        'dashboard_widgets',
        JSON.stringify(stubValue)
      );
      expect(value).toBe(stubValue);
      done();
    });
  });

  it('should returns settings from service with android', (done) => {
    platform.ANDROID = true;
    const stubValue: DashboardUserConfig = { widgets: [{ id: '12345' }] };
    jest.spyOn(rpcService, 'request').mockReturnValueOnce(of(stubValue));

    service.loadUserWidgetConfig().subscribe((value) => {
      expect(rpcService.request).toHaveBeenCalledTimes(1);
      expect(rpcService.request).toHaveBeenCalledWith('Dashboard', 'getDashboardSetting', {
        deviceType: 'mobile'
      });
      expect(value).toBe(stubValue);
      done();
    });
  });

  it('should returns settings from service with ios', (done) => {
    platform.IOS = true;
    const stubValue: DashboardUserConfig = { widgets: [{ id: '12345' }] };
    jest.spyOn(rpcService, 'request').mockReturnValueOnce(of(stubValue));

    service.loadUserWidgetConfig().subscribe((value) => {
      expect(rpcService.request).toHaveBeenCalledTimes(1);
      expect(rpcService.request).toHaveBeenCalledWith('Dashboard', 'getDashboardSetting', {
        deviceType: 'mobile'
      });
      expect(value).toBe(stubValue);
      done();
    });
  });

  it('should update service', (done) => {
    const stubValue: DashboardUserConfig = { widgets: [{ id: '12345' }] };
    jest
      .spyOn(rpcService, 'request')
      .mockReturnValueOnce(of(null))
      .mockReturnValueOnce(of(stubValue));

    jest
      .spyOn(userLocalStorageService, 'get')
      .mockReturnValueOnce(JSON.stringify(stubValue.widgets.map((m) => m.id)));

    jest.spyOn(userLocalStorageService, 'set');

    service.loadUserWidgetConfig().subscribe((value) => {
      expect(userLocalStorageService.get).toHaveBeenCalledTimes(1);
      expect(rpcService.request).toHaveBeenCalledTimes(2);
      expect(rpcService.request).toHaveBeenCalledWith('Dashboard', 'getDashboardSetting', {
        deviceType: 'desktop'
      });
      expect(rpcService.request).toHaveBeenCalledWith('Dashboard', 'setDashboardSetting', {
        deviceType: 'desktop',
        data: stubValue
      });
      expect(userLocalStorageService.set).toHaveBeenCalledTimes(1);
      expect(userLocalStorageService.set).toHaveBeenCalledWith(
        'dashboard_widgets',
        JSON.stringify(stubValue)
      );
      expect(value).toBe(stubValue);
      done();
    });
  });

  it('should not update local storage on error', (done) => {
    const stubValue: DashboardUserConfig = { widgets: [{ id: '12345' }] };
    jest.spyOn(rpcService, 'request').mockReturnValueOnce(throwError(new Error('blah')));

    jest
      .spyOn(userLocalStorageService, 'get')
      .mockReturnValueOnce(JSON.stringify(stubValue.widgets.map((m) => m.id)));

    jest.spyOn(userLocalStorageService, 'set');

    service.loadUserWidgetConfig().subscribe((value) => {
      expect(rpcService.request).toHaveBeenCalledTimes(1);
      expect(rpcService.request).toHaveBeenCalledWith('Dashboard', 'getDashboardSetting', {
        deviceType: 'desktop'
      });
      expect(userLocalStorageService.set).toHaveBeenCalledTimes(0);
      expect(value).toEqual({ widgets: [] });
      done();
    });
  });

  it('should set the settings', (done) => {
    const stubValue: DashboardUserConfig = { widgets: [{ id: '12345' }] };
    jest.spyOn(rpcService, 'request').mockReturnValueOnce(of(stubValue));

    service.setEnabled(stubValue.widgets.map((m) => m.id)).subscribe((value) => {
      expect(rpcService.request).toHaveBeenCalledWith('Dashboard', 'setDashboardSetting', {
        deviceType: 'desktop',
        data: stubValue
      });
      expect(value).toBe(stubValue);
      done();
    });
  });

  it('should reset the settings', (done) => {
    const stubValue: DashboardUserConfig = { widgets: [{ id: '12345' }] };
    jest.spyOn(rpcService, 'request').mockReturnValueOnce(of(stubValue));
    jest.spyOn(userLocalStorageService, 'remove');

    service.reset().subscribe((value) => {
      expect(rpcService.request).toHaveBeenCalledWith('Dashboard', 'deleteDashboardSetting', {
        deviceType: 'desktop'
      });
      expect(userLocalStorageService.remove).toHaveBeenCalledTimes(1);
      expect(value).toBe(stubValue);
      done();
    });
  });

  it('should return new format from getEnabled', (done) => {
    const stubValue: DashboardUserConfig = { widgets: [{ id: '12345' }] };
    jest.spyOn(userLocalStorageService, 'get').mockReturnValueOnce(JSON.stringify(stubValue));
    service.getEnabled().subscribe((value) => {
      expect(userLocalStorageService.get).toHaveBeenCalledTimes(1);
      expect(userLocalStorageService.get).toHaveBeenCalledWith(
        'dashboard_widgets',
        '{"widgets":[]}'
      );
      //expect(value).toEqual(stubValue);
      done();
    });
  });

  it('should return new format from getEnabled if old format in storage', (done) => {
    const expected: DashboardUserConfig = { widgets: [] };
    jest.spyOn(userLocalStorageService, 'get').mockReturnValueOnce('[]');
    service.getEnabled().subscribe((value) => {
      expect(userLocalStorageService.get).toHaveBeenCalledTimes(1);
      expect(value).toEqual(expected);
      done();
    });
  });
});
