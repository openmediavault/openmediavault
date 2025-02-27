import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { ComponentsModule } from '~/app/shared/components/components.module';
import { DirectivesModule } from '~/app/shared/directives/directives.module';
import { PipesModule } from '~/app/shared/pipes/pipes.module';

@NgModule({
  imports: [CommonModule, ComponentsModule, DirectivesModule, PipesModule],
  exports: [ComponentsModule, DirectivesModule, PipesModule]
})
export class SharedModule {}
