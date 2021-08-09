import { ErrorHandler, Injectable } from '@angular/core';
import { Router } from '@angular/router';

@Injectable({
  providedIn: 'root'
})
export class GlobalErrorHandlerService implements ErrorHandler {
  constructor(private router: Router) {}

  handleError(error: any): void {
    // Reload application if lazy loading of a module fails.
    if (/Loading chunk \d+ failed/.test(error?.message || '')) {
      this.router.navigate(['/reload']);
    }
    console.error(error);
  }
}
