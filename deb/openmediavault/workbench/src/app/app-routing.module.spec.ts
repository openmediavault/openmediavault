import { HttpClientTestingModule } from '@angular/common/http/testing';
import { TestBed } from '@angular/core/testing';
import { Router } from '@angular/router';
import { of } from 'rxjs';

import { AppRoutingModule } from '~/app/app-routing.module';
import { RouteConfigService } from '~/app/core/services/route-config.service';
import { RpcService } from '~/app/shared/services/rpc.service';

describe('AppRoutingModule', () => {
  let router: Router;
  let rpcService: RpcService;
  const downloadUrl = '/download?service=LogFile&method=getContent&params={"id":"syslog"}';

  beforeEach(async () => {
    TestBed.configureTestingModule({
      imports: [HttpClientTestingModule, AppRoutingModule],
      providers: [
        { provide: RpcService, useValue: { download: jest.fn() } },
        {
          provide: RouteConfigService,
          useValue: { load: () => of([]), injectPublicRoutes: jest.fn() }
        }
      ]
    });
    router = TestBed.inject(Router);
    rpcService = TestBed.inject(RpcService);
    await router.navigateByUrl('/503');
  });

  it('should execute the download if the navigation was triggered by the UI', async () => {
    await router.navigateByUrl(downloadUrl);
    expect(rpcService.download).toHaveBeenCalledWith('LogFile', 'getContent', { id: 'syslog' });
  });

  it('should not execute the download if the navigation was triggered externally [1]', async () => {
    // Simulate a manually entered URL or a click on a link.
    const spy = jest.spyOn(router, 'getCurrentNavigation');
    spy.mockReturnValue({ trigger: 'hashchange', previousNavigation: null } as any);
    await router.navigateByUrl(downloadUrl);
    expect(rpcService.download).not.toHaveBeenCalled();
    expect(router.url).toBe('/404');
  });

  it('should not execute the download if the navigation was triggered externally [2]', async () => {
    // Simulate the initial navigation, e.g. a bookmark or a page reload.
    const spy = jest.spyOn(router, 'getCurrentNavigation');
    spy.mockReturnValue({ trigger: 'imperative', previousNavigation: null } as any);
    await router.navigateByUrl(downloadUrl);
    expect(rpcService.download).not.toHaveBeenCalled();
    expect(router.url).toBe('/404');
  });
});
