import { HttpClient } from '@angular/common/http';
import { Injectable, NgModule } from '@angular/core';
import {
  DefaultTranspiler,
  Translation,
  TRANSLOCO_CONFIG,
  TRANSLOCO_LOADER,
  TRANSLOCO_TRANSPILER,
  translocoConfig,
  TranslocoLoader,
  TranslocoModule
} from '@ngneat/transloco';
import { HashMap } from '@ngneat/transloco/lib/types';
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

/**
 * Disable the build-in transpiler. String interpolation is done by
 * Nunjucks internally. Another reason is that missing keys are not
 * transpiled.
 */
@Injectable({ providedIn: 'root' })
class CustomTranspiler extends DefaultTranspiler {
  public override transpile(value: any, params: HashMap, translation: Translation): any {
    return value;
  }
}

@NgModule({
  exports: [TranslocoModule],
  providers: [
    {
      provide: TRANSLOCO_CONFIG,
      useValue: translocoConfig({
        availableLangs: _.keys(SupportedLocales),
        defaultLang: 'en_GB',
        missingHandler: { allowEmpty: true, logMissingKey: false },
        prodMode: environment.production,
        reRenderOnLangChange: true
      })
    },
    { provide: TRANSLOCO_LOADER, useClass: CustomLoader },
    { provide: TRANSLOCO_TRANSPILER, useClass: CustomTranspiler }
  ]
})
export class TranslocoRootModule {}
