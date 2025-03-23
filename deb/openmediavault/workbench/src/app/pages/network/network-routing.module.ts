import { NgModule } from '@angular/core';
import { RouterModule, ROUTES, Routes } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';

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
import { IsDirtyGuardService } from '~/app/shared/services/is-dirty-guard.service';

const routes: Routes = [
  {
    path: '',
    component: NavigationPageComponent
  },
  {
    path: 'general',
    component: GeneralNetworkFormPageComponent,
    canDeactivate: [IsDirtyGuardService],
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
        canDeactivate: [IsDirtyGuardService],
        data: {
          title: gettext('Create'),
          editing: false,
          notificationTitle: gettext('Created wired network connection.')
        }
      },
      {
        path: 'ethernet/edit/:uuid',
        component: InterfaceEthernetFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated wired network connection "{{ devicename }}".')
        }
      },
      {
        path: 'wifi/create',
        component: InterfaceWifiFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
        data: {
          title: gettext('Create'),
          editing: false,
          notificationTitle: gettext('Created wireless network connection.')
        }
      },
      {
        path: 'wifi/edit/:uuid',
        component: InterfaceWifiFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated wireless network connection "{{ devicename }}".')
        }
      },
      {
        path: 'bond/create',
        component: InterfaceBondFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
        data: {
          title: gettext('Create'),
          editing: false,
          notificationTitle: gettext('Created bond network connection.')
        }
      },
      {
        path: 'bond/edit/:uuid',
        component: InterfaceBondFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated bond network connection "{{ devicename }}".')
        }
      },
      {
        path: 'vlan/create',
        component: InterfaceVlanFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
        data: {
          title: gettext('Create'),
          editing: false,
          notificationTitle: gettext('Created VLAN network connection.')
        }
      },
      {
        path: 'vlan/edit/:uuid',
        component: InterfaceVlanFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated VLAN network connection "{{ devicename }}".')
        }
      },
      {
        path: 'bridge/create',
        component: InterfaceBridgeFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
        data: {
          title: gettext('Create'),
          editing: false,
          notificationTitle: gettext('Created bridge network connection.')
        }
      },
      {
        path: 'bridge/edit/:uuid',
        component: InterfaceBridgeFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
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
    canDeactivate: [IsDirtyGuardService],
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
              {
                path: 'inet',
                component: FirewallRuleInetDatatablePageComponent,
                canDeactivate: [IsDirtyGuardService]
              },
              {
                path: 'inet6',
                component: FirewallRuleInet6DatatablePageComponent,
                canDeactivate: [IsDirtyGuardService]
              }
            ]
          },
          {
            path: ':family/create',
            component: FirewallRuleFormPageComponent,
            canDeactivate: [IsDirtyGuardService],
            data: {
              title: gettext('Create'),
              editing: false,
              notificationTitle: gettext('Created firewall rule.')
            }
          },
          {
            path: ':family/edit/:uuid',
            component: FirewallRuleFormPageComponent,
            canDeactivate: [IsDirtyGuardService],
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
