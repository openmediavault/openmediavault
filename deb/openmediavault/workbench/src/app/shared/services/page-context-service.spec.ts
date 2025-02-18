import { provideLocationMocks } from '@angular/common/testing';
import { fakeAsync, TestBed } from '@angular/core/testing';
import { provideRouter } from '@angular/router';
import { RouterTestingHarness } from '@angular/router/testing';
import { expect } from '@jest/globals';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';

import { BlankLayoutComponent } from '~/app/core/components/layouts/blank-layout/blank-layout.component';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { PageContextService } from '~/app/shared/services/page-context-service';

// Test the correct working of the page context service
describe('PageContextService', () => {
  let authSessionService: AuthSessionService;
  let serviceUnderTest: PageContextService;
  let harness: RouterTestingHarness;

  beforeEach(fakeAsync(async () => {
    TestBed.configureTestingModule({
      providers: [
        // During development, I experienced some bugs only after switching from one page
        // to another. Therefore, I run all tests against two separate routes to make sure
        // the new values are applied correctly on page switching.
        provideRouter([
          {
            path: 'usermgmt/users/edit/:name',
            // I want to test the service itself here, not the integration
            // with specific components. That's why I choose the
            // BlankLayoutComponent here.
            component: BlankLayoutComponent,
            data: {
              title: gettext('Edit'),
              editing: true,
              notificationTitle: gettext('Updated user "{{ name }}".')
            }
          },
          {
            path: 'usermgmt/users/permissions/:secondName',
            component: BlankLayoutComponent,
            data: {
              title: gettext('Permissions'),
              breadcrumb: {
                text: '{{ "Permissions" | translate }} @ {{ _routeParams.secondName }}'
              },
              notificationTitle: gettext('Updated permissions of group "{{ name }}".')
            }
          }
        ]),
        provideLocationMocks(),
        AuthSessionService,
        PageContextService
      ]
    });

    harness = await RouterTestingHarness.create();
    authSessionService = TestBed.inject(AuthSessionService);
    serviceUnderTest = TestBed.inject(PageContextService);
  }));

  it('should be created', () => {
    expect(serviceUnderTest).toBeTruthy();
  });

  // The returned username field is a reference to the getUsername()
  // function. Therefore, we have to test the result immediately.
  it('should return the current username', () => {
    authSessionService.set('admin', { role: ['admin'] });
    const firstContext = serviceUnderTest.getContext();
    expect(firstContext._session.username).toEqual('admin');

    authSessionService.revoke();

    authSessionService.set('testUser', { role: ['user'] });
    const secondContext = serviceUnderTest.getContext();
    expect(secondContext._session.username).toEqual('testUser');
  });

  // The returned permissions field is a reference to the getPermissions()
  // function. Therefore, we have to test the result immediately.
  it('should return the current permissions', () => {
    authSessionService.set('admin', { role: ['admin'] });
    let context = serviceUnderTest.getContext();
    expect(context._session.permissions).toEqual({ role: ['admin'] });

    authSessionService.revoke();

    authSessionService.set('testUser', { role: ['user'] });
    context = serviceUnderTest.getContext();
    expect(context._session.permissions).toEqual({ role: ['user'] });
  });

  it('should return the correct route path', fakeAsync(async () => {
    await harness.navigateByUrl('usermgmt/users/edit/admin');
    const firstContext = serviceUnderTest.getContext();

    await harness.navigateByUrl('usermgmt/users/permissions/testUser');
    const secondContext = serviceUnderTest.getContext();

    expect(firstContext._routeConfig.path).toEqual('usermgmt/users/edit/:name');
    expect(secondContext._routeConfig.path).toEqual('usermgmt/users/permissions/:secondName');
  }));

  it('should return the correct route data', fakeAsync(async () => {
    await harness.navigateByUrl('usermgmt/users/edit/admin');
    const firstContext = serviceUnderTest.getContext();

    await harness.navigateByUrl('usermgmt/users/permissions/testUser');
    const secondContext = serviceUnderTest.getContext();

    expect(firstContext._routeConfig.data).toEqual({
      title: 'Edit',
      editing: true,
      notificationTitle: 'Updated user "{{ name }}".'
    });

    expect(secondContext._routeConfig.data).toEqual({
      title: 'Permissions',
      breadcrumb: {
        text: '{{ "Permissions" | translate }} @ {{ _routeParams.secondName }}'
      },
      notificationTitle: 'Updated permissions of group "{{ name }}".'
    });
  }));

  it('should return the correct editing status', fakeAsync(async () => {
    await harness.navigateByUrl('usermgmt/users/edit/admin');
    const firstContext = serviceUnderTest.getContext();

    await harness.navigateByUrl('usermgmt/users/permissions/testUser');
    const secondContext = serviceUnderTest.getContext();

    expect(firstContext._editing).toEqual(true);
    expect(secondContext._editing).toEqual(false);
  }));

  it('should return the correct url segments', fakeAsync(async () => {
    await harness.navigateByUrl('usermgmt/users/edit/admin');
    const firstContext = serviceUnderTest.getContext();

    await harness.navigateByUrl('usermgmt/users/permissions/testUser');
    const secondContext = serviceUnderTest.getContext();

    expect(firstContext._routeUrlSegments).toEqual(['usermgmt', 'users', 'edit', 'admin']);
    expect(secondContext._routeUrlSegments).toEqual([
      'usermgmt',
      'users',
      'permissions',
      'testUser'
    ]);
  }));

  it('should return the correct route parameters', fakeAsync(async () => {
    await harness.navigateByUrl('usermgmt/users/edit/admin');
    const firstContext = serviceUnderTest.getContext();

    await harness.navigateByUrl('usermgmt/users/permissions/testUser');
    const secondContext = serviceUnderTest.getContext();

    expect(firstContext._routeParams).toEqual({ name: 'admin' });
    expect(secondContext._routeParams).toEqual({ secondName: 'testUser' });
  }));

  it('should return the correct url query parameters', fakeAsync(async () => {
    await harness.navigateByUrl(
      'usermgmt/users/edit/admin?testValue1=true&testValue2=false&testValue3=&testValue4=%2Fthis%2Fis%5Ca%5Ctest%25'
    );
    const firstContext = serviceUnderTest.getContext();

    await harness.navigateByUrl('usermgmt/users/permissions/testUser');
    const secondContext = serviceUnderTest.getContext();

    expect(firstContext._routeQueryParams).toEqual({
      testValue1: 'true',
      testValue2: 'false',
      testValue3: '',
      testValue4: '/this/is\\a\\test%'
    });
    expect(secondContext._routeQueryParams).toEqual({});
  }));
});
