/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
import { HTTP_INTERCEPTORS, HttpClientModule } from '@angular/common/http';
import { APP_INITIALIZER, ErrorHandler, NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { TranslocoService } from '@ngneat/transloco';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import dayjs from 'dayjs';
import updateLocale from 'dayjs/plugin/updateLocale';
import { ToastrModule } from 'ngx-toastr';
import { tap } from 'rxjs/operators';

import { AppComponent } from '~/app/app.component';
import { AppRoutingModule } from '~/app/app-routing.module';
import { CoreModule } from '~/app/core/core.module';
import { getCurrentLocale, setTranslationService, translate } from '~/app/i18n.helper';
import { MaterialModule } from '~/app/material.module';
import { GlobalErrorHandlerService } from '~/app/shared/services/global-error-handler.service';
import { HttpErrorInterceptorService } from '~/app/shared/services/http-error-interceptor.service';
import { TitleService } from '~/app/shared/services/title.service';
import { SharedModule } from '~/app/shared/shared.module';
import { TranslocoRootModule } from '~/app/transloco-root.module';

@NgModule({
  declarations: [AppComponent],
  imports: [
    HttpClientModule,
    BrowserModule,
    BrowserAnimationsModule,
    CoreModule.forRoot(),
    SharedModule,
    ToastrModule.forRoot({
      positionClass: 'toast-bottom-center',
      preventDuplicates: true
    }),
    TranslocoRootModule,
    MaterialModule,
    AppRoutingModule
  ],
  providers: [
    {
      provide: ErrorHandler,
      useClass: GlobalErrorHandlerService
    },
    {
      provide: HTTP_INTERCEPTORS,
      useClass: HttpErrorInterceptorService,
      multi: true
    },
    {
      provide: APP_INITIALIZER,
      useFactory: (translocoService: TranslocoService) => () => {
        // Make translation service globally available to be able to use
        // it where DI is not possible.
        setTranslationService(translocoService);
        // Setup translation service. Delay app bootstrapping until
        // translation file has been loaded.
        translocoService.setActiveLang(getCurrentLocale());
        return translocoService
          .load(translocoService.getActiveLang())
          .pipe(
            tap(() => {
              // Translate various day.js locale strings.
              dayjs.extend(updateLocale);
              dayjs.updateLocale('en', {
                relativeTime: {
                  /* eslint-disable @typescript-eslint/naming-convention */
                  future: translate(gettext('in %s')),
                  past: translate(gettext('%s ago')),
                  s: translate(gettext('a few seconds')),
                  m: translate(gettext('a minute')),
                  mm: translate(gettext('%d minutes')),
                  h: translate(gettext('an hour')),
                  hh: translate(gettext('%d hours')),
                  d: translate(gettext('a day')),
                  dd: translate(gettext('%d days')),
                  M: translate(gettext('a month')),
                  MM: translate(gettext('%d months')),
                  y: translate(gettext('a year')),
                  yy: translate(gettext('%d years'))
                }
              });
            })
          )
          .toPromise();
      },
      multi: true,
      deps: [TranslocoService]
    }
  ],
  bootstrap: [AppComponent]
})
export class AppModule {
  constructor(public titleService: TitleService) {}
}
