import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { ComponentsModule } from '~/app/shared/components/components.module';
import { DirectivesModule } from '~/app/shared/directives/directives.module';
import { PipesModule } from '~/app/shared/pipes/pipes.module';
import { AuthGuardService } from '~/app/shared/services/auth-guard.service';
import { SystemInformationService } from '~/app/shared/services/system-information.service';

@NgModule({
  imports: [CommonModule, ComponentsModule, DirectivesModule, PipesModule],
  exports: [ComponentsModule, DirectivesModule, PipesModule],
  providers: [AuthGuardService, SystemInformationService]
})
export class SharedModule {}
