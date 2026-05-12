import { fakeAsync, TestBed } from '@angular/core/testing';
import { provideRouter } from '@angular/router';
import { RouterTestingHarness } from '@angular/router/testing';

import { BlankLayoutComponent } from '~/app/core/components/layouts/blank-layout/blank-layout.component';
import { PageContextService } from '~/app/core/services/page-context.service';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { TestingModule } from '~/app/testing.module';

describe('PageContextService', () => {
  let authSessionService: AuthSessionService;
  let pageContextService: PageContextService;
  let routerTestingHarness: RouterTestingHarness;

  beforeEach(fakeAsync(async () => {
    TestBed.configureTestingModule({
      imports: [TestingModule],
      providers: [
        PageContextService,
        provideRouter([
          {
            path: 'usermgmt/users/edit/:name',
            component: BlankLayoutComponent,
            data: {
              title: 'Edit',
              editing: true,
              notificationTitle: 'Updated user "{{ name }}".'
            }
          }
        ])
      ]
    });
    authSessionService = TestBed.inject(AuthSessionService);
    authSessionService.set('foo', { role: ['user'] });
    routerTestingHarness = await RouterTestingHarness.create();
    pageContextService = TestBed.inject(PageContextService);
  }));

  it('should be created', () => {
    expect(pageContextService).toBeTruthy();
  });

  it('should return the current session information', () => {
    expect(pageContextService.get()._session.username).toEqual('foo');
    expect(pageContextService.get()._session.permissions).toEqual({ role: ['user'] });
  });

  it('should return the current route configuration', fakeAsync(async () => {
    await routerTestingHarness.navigateByUrl('usermgmt/users/edit/bar');
    const ctx = pageContextService.get();
    expect(ctx._routeConfig.path).toEqual('usermgmt/users/edit/:name');
    expect(ctx._routeConfig.data).toEqual({
      title: 'Edit',
      editing: true,
      notificationTitle: 'Updated user "{{ name }}".'
    });
    expect(ctx._routeParams).toEqual({ name: 'bar' });
    expect(ctx._routeUrlSegments).toEqual(['usermgmt', 'users', 'edit', 'bar']);
  }));
});
