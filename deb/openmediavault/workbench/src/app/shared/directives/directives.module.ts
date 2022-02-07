import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { AutocapitalizeDirective } from '~/app/shared/directives/autocapitalize.directive';
import { AutofocusDirective } from '~/app/shared/directives/autofocus.directive';
import { BinaryUnitDirective } from '~/app/shared/directives/binary-unit.directive';
import { NativeElementDirective } from '~/app/shared/directives/native-element.directive';
import { StickyDirective } from '~/app/shared/directives/sticky.directive';

@NgModule({
  declarations: [
    AutocapitalizeDirective,
    AutofocusDirective,
    NativeElementDirective,
    StickyDirective,
    BinaryUnitDirective
  ],
  exports: [
    AutocapitalizeDirective,
    AutofocusDirective,
    NativeElementDirective,
    StickyDirective,
    BinaryUnitDirective
  ],
  imports: [CommonModule]
})
export class DirectivesModule {}
