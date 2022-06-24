import { HttpClient } from '@angular/common/http';
import { Injectable, NgModule } from '@angular/core';
import {
  Translation,
  TRANSLOCO_CONFIG,
  TRANSLOCO_LOADER,
  translocoConfig,
  TranslocoLoader,
  TranslocoModule
} from '@ngneat/transloco';
import * as _ from 'lodash';
import { Observable, of } from 'rxjs';
import { catchError } from 'rxjs/operators';

import { Locale } from '~/app/shared/enum/locale.enum';
import { environment } from '~/environments/environment';

@Injectable({ providedIn: 'root' })
export class TranslocoHttpLoader implements TranslocoLoader {
  constructor(private http: HttpClient) {}

  public getTranslation(lang: string): Observable<Translation> {
    return this.http.get<Translation>(`/assets/i18n/${lang}.json`).pipe(
      catchError((error) => {
        // Do not show an error notification and return an empty language
        // dictionary in case of the translation file does not exist.
        if (_.isFunction(error.preventDefault)) {
          error.preventDefault();
        }
        return of({});
      })
    );
  }
}

@NgModule({
  exports: [TranslocoModule],
  providers: [
    {
      provide: TRANSLOCO_CONFIG,
      useValue: translocoConfig({
        availableLangs: _.keys(Locale),
        defaultLang: 'en_GB',
        missingHandler: { allowEmpty: true, logMissingKey: false },
        prodMode: environment.production,
        reRenderOnLangChange: true
      })
    },
    { provide: TRANSLOCO_LOADER, useClass: TranslocoHttpLoader }
  ]
})
export class TranslocoRootModule {}
