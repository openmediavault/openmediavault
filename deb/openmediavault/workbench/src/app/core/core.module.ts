import { CommonModule } from '@angular/common';
import { ModuleWithProviders, NgModule } from '@angular/core';

import { ComponentsModule } from '~/app/core/components/components.module';
import { PagesModule } from '~/app/core/pages/pages.module';
import { DashboardWidgetConfigService } from '~/app/core/services/dashboard-widget-config.service';
import { LogConfigService } from '~/app/core/services/log-config.service';
import { NavigationConfigService } from '~/app/core/services/navigation-config.service';
import { RouteConfigService } from '~/app/core/services/route-config.service';

@NgModule({
  declarations: [],
  imports: [CommonModule, ComponentsModule, PagesModule],
  exports: [ComponentsModule, PagesModule]
})
export class CoreModule {
  static forRoot(): ModuleWithProviders<CoreModule> {
    return {
      ngModule: CoreModule,
      providers: [
        // These services must be singletons across the whole app.
        DashboardWidgetConfigService,
        LogConfigService,
        NavigationConfigService,
        RouteConfigService
      ]
    };
  }
}
