import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { CoreModule } from '~/app/core/core.module';
import { AccountPasswordFormPageComponent } from '~/app/pages/account/account-password-form-page.component';
import { AccountRoutingModule } from '~/app/pages/account/account-routing.module';
import { AccountSettingsFormPageComponent } from '~/app/pages/account/account-settings-form-page.component';

@NgModule({
  declarations: [AccountPasswordFormPageComponent, AccountSettingsFormPageComponent],
  imports: [CommonModule, CoreModule, AccountRoutingModule]
})
export class AccountModule {}
