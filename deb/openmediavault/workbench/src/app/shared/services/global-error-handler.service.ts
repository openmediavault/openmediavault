import { ErrorHandler, Injectable } from '@angular/core';
import { Router } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

@Injectable({
  providedIn: 'root'
})
export class GlobalErrorHandlerService implements ErrorHandler {
  constructor(private router: Router) {}

  handleError(error: any): void {
    console.error(error);
    // Reload application if lazy loading of a module fails.
    if (/Loading chunk \d+ failed/.test(error?.message || '')) {
      this.router.navigate(['/guruMeditation'], {
        queryParams: {
          message: gettext('Obsolete cached data found. Page will be reloaded.'),
          url: '/reload'
        }
      });
    }
  }
}
