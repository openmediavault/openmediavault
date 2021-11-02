import { NgModule } from '@angular/core';
import { RouterModule, ROUTES, Routes } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { NavigationPageComponent } from '~/app/core/pages/navigation-page/navigation-page.component';
import { RouteConfigService } from '~/app/core/services/route-config.service';
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
import { ProxyFormPageComponent } from '~/app/pages/network/proxy/proxy-form-page.component';

const routes: Routes = [
  {
    path: '',
    component: NavigationPageComponent
  },
  {
    path: 'general',
    component: GeneralNetworkFormPageComponent,
    data: {
      title: gettext('General'),
      notificationTitle: gettext('Updated network settings.'),
      editing: true
    }
  },
  {
    path: 'interfaces',
    data: { title: gettext('Interfaces') },
    children: [
      { path: '', component: InterfaceDatatablePageComponent },
      {
        path: 'details/:devicename',
        component: InterfaceDetailsFormPageComponent,
        data: { title: gettext('Details'), editing: true }
      },
      {
        path: 'ethernet/create',
        component: InterfaceEthernetFormPageComponent,
        data: {
          title: gettext('Create'),
          editing: false,
          notificationTitle: gettext('Created wired network connection.')
        }
      },
      {
        path: 'ethernet/edit/:uuid',
        component: InterfaceEthernetFormPageComponent,
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated wired network connection "{{ devicename }}".')
        }
      },
      {
        path: 'wireless/create',
        component: InterfaceWifiFormPageComponent,
        data: {
          title: gettext('Create'),
          editing: false,
          notificationTitle: gettext('Created wireless network connection.')
        }
      },
      {
        path: 'wireless/edit/:uuid',
        component: InterfaceWifiFormPageComponent,
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated wireless network connection "{{ devicename }}".')
        }
      },
      {
        path: 'bond/create',
        component: InterfaceBondFormPageComponent,
        data: {
          title: gettext('Create'),
          editing: false,
          notificationTitle: gettext('Created bond network connection.')
        }
      },
      {
        path: 'bond/edit/:uuid',
        component: InterfaceBondFormPageComponent,
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated bond network connection "{{ devicename }}".')
        }
      },
      {
        path: 'vlan/create',
        component: InterfaceVlanFormPageComponent,
        data: {
          title: gettext('Create'),
          editing: false,
          notificationTitle: gettext('Created VLAN network connection.')
        }
      },
      {
        path: 'vlan/edit/:uuid',
        component: InterfaceVlanFormPageComponent,
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated VLAN network connection "{{ devicename }}".')
        }
      },
      {
        path: 'bridge/create',
        component: InterfaceBridgeFormPageComponent,
        data: {
          title: gettext('Create'),
          editing: false,
          notificationTitle: gettext('Created bridge network connection.')
        }
      },
      {
        path: 'bridge/edit/:uuid',
        component: InterfaceBridgeFormPageComponent,
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated bridge network connection "{{ devicename }}".')
        }
      }
    ]
  },
  {
    path: 'proxy',
    component: ProxyFormPageComponent,
    data: {
      title: gettext('Proxy'),
      notificationTitle: gettext('Updated proxy settings.'),
      editing: true
    }
  },
  {
    path: 'firewall',
    data: { title: gettext('Firewall') },
    children: [
      { path: '', component: NavigationPageComponent },
      {
        path: 'rules',
        data: { title: gettext('Rules') },
        children: [
          {
            path: '',
            component: FirewallRuleTabsPageComponent,
            children: [
              {
                path: '',
                redirectTo: 'inet',
                pathMatch: 'full'
              },
              { path: 'inet', component: FirewallRuleInetDatatablePageComponent },
              { path: 'inet6', component: FirewallRuleInet6DatatablePageComponent }
            ]
          },
          {
            path: ':family/create',
            component: FirewallRuleFormPageComponent,
            data: {
              title: gettext('Create'),
              editing: false,
              notificationTitle: gettext('Created firewall rule.')
            }
          },
          {
            path: ':family/edit/:uuid',
            component: FirewallRuleFormPageComponent,
            data: {
              title: gettext('Edit'),
              editing: true,
              notificationTitle: gettext('Updated firewall rule.')
            }
          }
        ]
      }
    ]
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
        routeConfigService.inject('network', routes);
        return routes;
      },
      deps: [RouteConfigService]
    }
  ]
})
export class NetworkRoutingModule {}
