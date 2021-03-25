import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { AutofocusDirective } from '~/app/shared/directives/autofocus.directive';
import { NativeElementDirective } from '~/app/shared/directives/native-element.directive';
import { StickyDirective } from '~/app/shared/directives/sticky.directive';

@NgModule({
  declarations: [AutofocusDirective, NativeElementDirective, StickyDirective],
  exports: [AutofocusDirective, NativeElementDirective, StickyDirective],
  imports: [CommonModule]
})
export class DirectivesModule {}
