import { APP_INITIALIZER, NgModule } from '@angular/core';
import { ActivatedRouteSnapshot, RouterModule, Routes } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';
import { EMPTY } from 'rxjs';

import { BlankLayoutComponent } from '~/app/core/components/layouts/blank-layout/blank-layout.component';
import { WorkbenchLayoutComponent } from '~/app/core/components/layouts/workbench-layout/workbench-layout.component';
import { AboutPageComponent } from '~/app/core/pages/about-page/about-page.component';
import { EmptyPageComponent } from '~/app/core/pages/empty-page/empty-page.component';
import { GuruMeditationPageComponent } from '~/app/core/pages/guru-meditation-page/guru-meditation-page.component';
import { LoginPageComponent } from '~/app/core/pages/login-page/login-page.component';
import { NavigationPageComponent } from '~/app/core/pages/navigation-page/navigation-page.component';
import { ShutdownPageComponent } from '~/app/core/pages/shutdown-page/shutdown-page.component';
import { StandbyPageComponent } from '~/app/core/pages/standby-page/standby-page.component';
import { RouteConfigService } from '~/app/core/services/route-config.service';
import { AuthGuardService } from '~/app/shared/services/auth-guard.service';

const routes: Routes = [
  { path: '', redirectTo: 'dashboard', pathMatch: 'full' },
  {
    path: '',
    component: WorkbenchLayoutComponent,
    children: [
      {
        path: '',
        canActivate: [AuthGuardService],
        canActivateChild: [AuthGuardService],
        component: NavigationPageComponent
      },
      { path: 'about', component: AboutPageComponent },
      {
        path: 'dashboard',
        canActivate: [AuthGuardService],
        canActivateChild: [AuthGuardService],
        loadChildren: () =>
          import('./pages/dashboard/dashboard.module').then((m) => m.DashboardModule),
        data: { title: gettext('Dashboard') }
      },
      {
        path: 'system',
        canActivate: [AuthGuardService],
        canActivateChild: [AuthGuardService],
        loadChildren: () => import('./pages/system/system.module').then((m) => m.SystemModule),
        data: { title: gettext('System') }
      },
      {
        path: 'usermgmt',
        canActivate: [AuthGuardService],
        canActivateChild: [AuthGuardService],
        loadChildren: () =>
          import('./pages/usermgmt/usermgmt.module').then((m) => m.UsermgmtModule),
        data: { title: gettext('User Management') }
      },
      {
        path: 'network',
        canActivate: [AuthGuardService],
        canActivateChild: [AuthGuardService],
        loadChildren: () => import('./pages/network/network.module').then((m) => m.NetworkModule),
        data: { title: gettext('Network') }
      },
      {
        path: 'storage',
        canActivate: [AuthGuardService],
        canActivateChild: [AuthGuardService],
        loadChildren: () => import('./pages/storage/storage.module').then((m) => m.StorageModule),
        data: { title: gettext('Storage') }
      },
      {
        path: 'services',
        canActivate: [AuthGuardService],
        canActivateChild: [AuthGuardService],
        loadChildren: () =>
          import('./pages/services/services.module').then((m) => m.ServicesModule),
        data: { title: gettext('Services') }
      },
      {
        path: 'diagnostics',
        canActivate: [AuthGuardService],
        canActivateChild: [AuthGuardService],
        loadChildren: () =>
          import('./pages/diagnostics/diagnostics.module').then((m) => m.DiagnosticsModule),
        data: { title: gettext('Diagnostics') }
      },
      { path: 'logout', children: [] }
    ]
  },
  {
    path: '',
    component: BlankLayoutComponent,
    children: [
      { path: 'login', component: LoginPageComponent },
      { path: 'standby', component: StandbyPageComponent },
      { path: 'shutdown', component: ShutdownPageComponent },
      {
        path: 'externalRedirect/:url',
        resolve: {
          url: 'externalRedirectResolver'
        },
        component: EmptyPageComponent
      },
      {
        path: 'reload',
        resolve: {
          url: 'reloadResolver'
        },
        component: EmptyPageComponent
      },
      {
        path: '404',
        component: GuruMeditationPageComponent,
        data: { message: gettext('The requested page was not found.') }
      },
      {
        path: '503',
        component: GuruMeditationPageComponent,
        data: { message: gettext('The server is unavailable to handle this request right now.') }
      }
    ]
  },
  { path: '**', redirectTo: '/404' }
];

@NgModule({
  exports: [RouterModule],
  imports: [
    RouterModule.forRoot(routes, {
      useHash: true
    })
  ],
  providers: [
    RouteConfigService,
    {
      provide: APP_INITIALIZER,
      useFactory: (routeConfigService: RouteConfigService) => (): Promise<Routes> =>
        // Make sure custom routes are loaded while bootstrapping.
        // This way we can inject custom routes in during lazy loading.
        routeConfigService.load().toPromise(),
      multi: true,
      deps: [RouteConfigService]
    },
    {
      provide: 'externalRedirectResolver',
      useValue: (route: ActivatedRouteSnapshot) => {
        // Get the external URL to redirect to.
        // Example: /externalRedirect/https%3A%2F%2Fwww.openmediavault.org
        const url = decodeURIComponent(route.paramMap.get('url'));
        if (_.isString(url)) {
          window.open(url, '_blank');
        }
        return EMPTY;
      }
    },
    {
      provide: 'reloadResolver',
      useValue: () => {
        // Reload the whole page.
        document.location.replace('');
        return EMPTY;
      }
    }
  ]
})
export class AppRoutingModule {}
