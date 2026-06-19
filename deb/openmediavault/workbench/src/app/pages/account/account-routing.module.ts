import { NgModule } from '@angular/core';
import { RouterModule, ROUTES, Routes } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';

import { NavigationPageComponent } from '~/app/core/pages/navigation-page/navigation-page.component';
import { RouteConfigService } from '~/app/core/services/route-config.service';
import { AccountPasswordFormPageComponent } from '~/app/pages/account/account-password-form-page.component';
import { AccountSettingsFormPageComponent } from '~/app/pages/account/account-settings-form-page.component';
import { IsDirtyGuardService } from '~/app/shared/services/is-dirty-guard.service';

const routes: Routes = [
  {
    path: '',
    component: NavigationPageComponent
  },
  {
    path: 'password',
    component: AccountPasswordFormPageComponent,
    canDeactivate: [IsDirtyGuardService],
    data: {
      title: gettext('Password'),
      editing: true,
      notificationTitle: gettext('Updated password.')
    }
  },
  {
    path: 'settings',
    component: AccountSettingsFormPageComponent,
    canDeactivate: [IsDirtyGuardService],
    data: {
      title: gettext('Settings'),
      editing: true,
      notificationTitle: gettext('Updated user settings.')
    }
  }
];

@NgModule({
  exports: [RouterModule],
  imports: [RouterModule.forChild(routes)],
  providers: [
    {
      provide: ROUTES,
      multi: true,
      useFactory: (routeConfigService: RouteConfigService): Routes => {
        routeConfigService.injectWorkbenchRoutes('account', routes);
        return routes;
      },
      deps: [RouteConfigService]
    }
  ]
})
export class AccountRoutingModule {}
