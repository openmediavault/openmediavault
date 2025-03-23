import { ErrorHandler, Injectable } from '@angular/core';
import { Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';

@Injectable({
  providedIn: 'root'
})
export class GlobalErrorHandlerService implements ErrorHandler {
  constructor(private router: Router) {}

  handleError(error: any): void {
    // Reload application if lazy loading of a module fails.
    if (/Loading chunk \d+ failed/.test(error?.message || '')) {
      this.router.navigate(['/guruMeditation'], {
        queryParams: {
          message: gettext(
            'Obsolete cached data found. Page will be reloaded. If the problem persists, press Ctrl-Shift-R or Ctrl-F5 to force a hard reload of the page.'
          ),
          url: '/reload'
        }
      });
    }
  }
}
