import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { TranslocoModule } from '@ngneat/transloco';

import { CoreModule } from '~/app/core/core.module';
import { MaterialModule } from '~/app/material.module';
import { FirewallRuleFormPageComponent } from '~/app/pages/network/firewall/rules/firewall-rule-form-page.component';
import { FirewallRuleInetDatatablePageComponent } from '~/app/pages/network/firewall/rules/firewall-rule-inet-datatable-page.component';
import { FirewallRuleInet6DatatablePageComponent } from '~/app/pages/network/firewall/rules/firewall-rule-inet6-datatable-page.component';
import { FirewallRuleTabsPageComponent } from '~/app/pages/network/firewall/rules/firewall-rule-tabs-page.component';
import { GeneralNetworkFormPageComponent } from '~/app/pages/network/general/general-network-form-page.component';
import { InterfaceBondFormPageComponent } from '~/app/pages/network/interfaces/interface-bond-form-page.component';
import { InterfaceBridgeFormPageComponent } from '~/app/pages/network/interfaces/interface-bridge-form-page.component';
import { InterfaceDatatablePageComponent } from '~/app/pages/network/interfaces/interface-datatable-page.component';
import { InterfaceDetailsFormPageComponent } from '~/app/pages/network/interfaces/interface-details-form-page.component';
import { InterfaceEthernetFormPageComponent } from '~/app/pages/network/interfaces/interface-ethernet-form-page.component';
import { InterfaceVlanFormPageComponent } from '~/app/pages/network/interfaces/interface-vlan-form-page.component';
import { InterfaceWifiFormPageComponent } from '~/app/pages/network/interfaces/interface-wifi-form-page.component';
import { NetworkRoutingModule } from '~/app/pages/network/network-routing.module';
import { ProxyFormPageComponent } from '~/app/pages/network/proxy/proxy-form-page.component';
import { SharedModule } from '~/app/shared/shared.module';

@NgModule({
  declarations: [
    GeneralNetworkFormPageComponent,
    ProxyFormPageComponent,
    InterfaceDatatablePageComponent,
    InterfaceEthernetFormPageComponent,
    InterfaceWifiFormPageComponent,
    InterfaceBondFormPageComponent,
    InterfaceVlanFormPageComponent,
    FirewallRuleFormPageComponent,
    FirewallRuleTabsPageComponent,
    FirewallRuleInetDatatablePageComponent,
    FirewallRuleInet6DatatablePageComponent,
    InterfaceBridgeFormPageComponent,
    InterfaceDetailsFormPageComponent
  ],
  imports: [
    CommonModule,
    CoreModule,
    MaterialModule,
    SharedModule,
    NetworkRoutingModule,
    TranslocoModule
  ]
})
export class NetworkModule {}
