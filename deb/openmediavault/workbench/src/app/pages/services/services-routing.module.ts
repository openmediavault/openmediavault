import { NgModule } from '@angular/core';
import { RouterModule, ROUTES, Routes } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { NavigationPageComponent } from '~/app/core/pages/navigation-page/navigation-page.component';
import { RouteConfigService } from '~/app/core/services/route-config.service';
import { FtpBanRuleDatatablePageComponent } from '~/app/pages/services/ftp/ftp-ban-rule-datatable-page.component';
import { FtpBanRuleFormPageComponent } from '~/app/pages/services/ftp/ftp-ban-rule-form-page.component';
import { FtpSettingsFormPageComponent } from '~/app/pages/services/ftp/ftp-settings-form-page.component';
import { FtpShareDatatablePageComponent } from '~/app/pages/services/ftp/ftp-share-datatable-page.component';
import { FtpShareFormPageComponent } from '~/app/pages/services/ftp/ftp-share-form-page.component';
import { FtpTlsSettingsFormPageComponent } from '~/app/pages/services/ftp/ftp-tls-settings-form-page.component';
import { NfsSettingsFormPageComponent } from '~/app/pages/services/nfs/nfs-settings-form-page.component';
import { NfsShareDatatablePageComponent } from '~/app/pages/services/nfs/nfs-share-datatable-page.component';
import { NfsShareFormPageComponent } from '~/app/pages/services/nfs/nfs-share-form-page.component';
import { RsyncModuleDatatablePageComponent } from '~/app/pages/services/rsync/rsync-module-datatable-page.component';
import { RsyncModuleFormPageComponent } from '~/app/pages/services/rsync/rsync-module-form-page.component';
import { RsyncModuleSettingsFormPageComponent } from '~/app/pages/services/rsync/rsync-module-settings-form-page.component';
import { RsyncTaskDatatablePageComponent } from '~/app/pages/services/rsync/rsync-task-datatable-page.component';
import { RsyncTaskFormPageComponent } from '~/app/pages/services/rsync/rsync-task-form-page.component';
import { SmbSettingsFormPageComponent } from '~/app/pages/services/smb/smb-settings-form-page.component';
import { SmbShareDatatablePageComponent } from '~/app/pages/services/smb/smb-share-datatable-page.component';
import { SmbShareFormPageComponent } from '~/app/pages/services/smb/smb-share-form-page.component';
import { SshFormPageComponent } from '~/app/pages/services/ssh/ssh-form-page.component';

const routes: Routes = [
  {
    path: '',
    component: NavigationPageComponent
  },
  {
    path: 'ssh',
    component: SshFormPageComponent,
    data: {
      title: gettext('SSH'),
      editing: true,
      notificationTitle: gettext('Updated SSH settings.')
    }
  },
  {
    path: 'ftp',
    data: { title: gettext('FTP') },
    children: [
      { path: '', component: NavigationPageComponent },
      {
        path: 'settings',
        component: FtpSettingsFormPageComponent,
        data: {
          title: gettext('Settings'),
          editing: true,
          notificationTitle: gettext('Updated FTP settings.')
        }
      },
      {
        path: 'tls-settings',
        component: FtpTlsSettingsFormPageComponent,
        data: {
          title: gettext('SSL/TLS Settings'),
          editing: true,
          notificationTitle: gettext('Updated FTP SSL/TLS settings.')
        }
      },
      {
        path: 'ban-rules',
        data: { title: gettext('Ban Rules') },
        children: [
          { path: '', component: FtpBanRuleDatatablePageComponent },
          {
            path: 'create',
            component: FtpBanRuleFormPageComponent,
            data: {
              title: gettext('Create'),
              editing: false,
              notificationTitle: gettext('Created FTP ban rule.')
            }
          },
          {
            path: 'edit/:uuid',
            component: FtpBanRuleFormPageComponent,
            data: {
              title: gettext('Edit'),
              editing: true,
              notificationTitle: gettext('Updated FTP ban rule.')
            }
          }
        ]
      },
      {
        path: 'shares',
        data: { title: gettext('Shares') },
        children: [
          { path: '', component: FtpShareDatatablePageComponent },
          {
            path: 'create',
            component: FtpShareFormPageComponent,
            data: {
              title: gettext('Create'),
              editing: false,
              notificationTitle: gettext('Created FTP share.')
            }
          },
          {
            path: 'edit/:uuid',
            component: FtpShareFormPageComponent,
            data: {
              title: gettext('Edit'),
              editing: true,
              notificationTitle: gettext('Updated FTP share.')
            }
          }
        ]
      }
    ]
  },
  {
    path: 'rsync',
    data: { title: gettext('Rsync') },
    children: [
      { path: '', component: NavigationPageComponent },
      {
        path: 'tasks',
        data: { title: gettext('Tasks') },
        children: [
          { path: '', component: RsyncTaskDatatablePageComponent },
          {
            path: 'create',
            component: RsyncTaskFormPageComponent,
            data: {
              title: gettext('Create'),
              editing: false,
              notificationTitle: gettext('Created Rsync task.')
            }
          },
          {
            path: 'edit/:uuid',
            component: RsyncTaskFormPageComponent,
            data: {
              title: gettext('Edit'),
              editing: true,
              notificationTitle: gettext('Updated Rsync task.')
            }
          }
        ]
      },
      {
        path: 'server',
        data: { title: gettext('Server') },
        children: [
          { path: '', component: NavigationPageComponent },
          {
            path: 'settings',
            component: RsyncModuleSettingsFormPageComponent,
            data: {
              title: gettext('Settings'),
              editing: true,
              notificationTitle: gettext('Updated Rsync Server settings.')
            }
          },
          {
            path: 'modules',
            data: { title: gettext('Modules') },
            children: [
              { path: '', component: RsyncModuleDatatablePageComponent },
              {
                path: 'create',
                component: RsyncModuleFormPageComponent,
                data: {
                  title: gettext('Create'),
                  editing: false,
                  notificationTitle: gettext('Created Rsync server module "{{ name }}".')
                }
              },
              {
                path: 'edit/:uuid',
                component: RsyncModuleFormPageComponent,
                data: {
                  title: gettext('Edit'),
                  editing: true,
                  notificationTitle: gettext('Updated Rsync server module "{{ name }}".')
                }
              }
            ]
          }
        ]
      }
    ]
  },
  {
    path: 'smb',
    data: { title: gettext('SMB/CIFS') },
    children: [
      { path: '', component: NavigationPageComponent },
      {
        path: 'settings',
        component: SmbSettingsFormPageComponent,
        data: {
          title: gettext('Settings'),
          editing: true,
          notificationTitle: gettext('Updated SMB/CIFS settings.')
        }
      },
      {
        path: 'shares',
        data: { title: gettext('Shares') },
        children: [
          { path: '', component: SmbShareDatatablePageComponent },
          {
            path: 'create',
            component: SmbShareFormPageComponent,
            data: {
              title: gettext('Create'),
              editing: false,
              notificationTitle: gettext('Created SMB/CIFS share.')
            }
          },
          {
            path: 'edit/:uuid',
            component: SmbShareFormPageComponent,
            data: {
              title: gettext('Edit'),
              editing: true,
              notificationTitle: gettext('Updated SMB/CIFS share.')
            }
          }
        ]
      }
    ]
  },
  {
    path: 'nfs',
    data: { title: gettext('NFS') },
    children: [
      { path: '', component: NavigationPageComponent },
      {
        path: 'settings',
        component: NfsSettingsFormPageComponent,
        data: {
          title: gettext('Settings'),
          editing: true,
          notificationTitle: gettext('Updated NFS settings.')
        }
      },
      {
        path: 'shares',
        data: { title: gettext('Shares') },
        children: [
          { path: '', component: NfsShareDatatablePageComponent },
          {
            path: 'create',
            component: NfsShareFormPageComponent,
            data: {
              title: gettext('Create'),
              editing: false,
              notificationTitle: gettext('Created NFS share.')
            }
          },
          {
            path: 'edit/:uuid',
            component: NfsShareFormPageComponent,
            data: {
              title: gettext('Edit'),
              editing: true,
              notificationTitle: gettext('Updated NFS share.')
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
        routeConfigService.inject('services', routes);
        return routes;
      },
      deps: [RouteConfigService]
    }
  ]
})
export class ServicesRoutingModule {}
