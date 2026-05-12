import { NgModule } from '@angular/core';
import { RouterModule, ROUTES, Routes } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';

import { DashboardPageComponent } from '~/app/core/pages/dashboard-page/dashboard-page.component';
import { RouteConfigService } from '~/app/core/services/route-config.service';
import { DashboardSettingsPageComponent } from '~/app/pages/dashboard/dashboard-settings-page/dashboard-settings-page.component';
import { IsDirtyGuardService } from '~/app/shared/services/is-dirty-guard.service';

const routes: Routes = [
  {
    path: '',
    component: DashboardPageComponent,
    data: { title: gettext('Dashboard') }
  },
  {
    path: 'settings',
    component: DashboardSettingsPageComponent,
    canDeactivate: [IsDirtyGuardService],
    data: { title: gettext('Settings'), notificationTitle: gettext('Updated dashboard settings.') }
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
        routeConfigService.inject('dashboard', routes);
        return routes;
      },
      deps: [RouteConfigService]
    }
  ]
})
export class DashboardRoutingModule {}
