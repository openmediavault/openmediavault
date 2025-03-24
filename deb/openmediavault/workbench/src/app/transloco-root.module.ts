import { HttpClient } from '@angular/common/http';
import { Injectable, NgModule } from '@angular/core';
import {
  Translation,
  TRANSLOCO_LOADER,
  TranslocoLoader,
  FunctionalTranspiler, provideTranslocoTranspiler,
  provideTransloco,
  TranslocoModule
} from '@jsverse/transloco';
import * as _ from 'lodash';
import { Observable, of } from 'rxjs';
import { catchError } from 'rxjs/operators';

import { SupportedLocales } from '~/app/i18n.helper';
import { environment } from '~/environments/environment';

/**
 * Load the translation dictionary.
 */
@Injectable({ providedIn: 'root' })
class CustomLoader implements TranslocoLoader {
  constructor(private http: HttpClient) {}

  public getTranslation(lang: string): Observable<Translation> {
    return this.http.get<Translation>(`./assets/i18n/${lang}.json`).pipe(
      catchError((error) => {
        // Do not show an error notification and return an empty language
        // dictionary in case of the translation file does not exist.
        error.preventDefault?.();
        return of({});
      })
    );
  }
}

@NgModule({
  exports: [TranslocoModule],
  providers: [
    provideTranslocoTranspiler(FunctionalTranspiler),
    provideTransloco({
      config: {
        availableLangs: _.keys(SupportedLocales),
        defaultLang: 'en_GB',
        missingHandler: { allowEmpty: true, logMissingKey: false },
        prodMode: environment.production,
        reRenderOnLangChange: true
       }
      }),
    { provide: TRANSLOCO_LOADER, useClass: CustomLoader }
  ]
})
export class TranslocoRootModule {}
