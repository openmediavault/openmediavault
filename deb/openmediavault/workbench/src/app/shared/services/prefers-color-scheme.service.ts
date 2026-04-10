import { Injectable } from '@angular/core';
import { BehaviorSubject, Observable } from 'rxjs';

import { UserLocalStorageService } from '~/app/shared/services/user-local-storage.service';

export type PrefersColorScheme = 'light' | 'dark';
export const DEFAULT_COLORSCHEME: PrefersColorScheme = 'light';

@Injectable({
  providedIn: 'root'
})
export class PrefersColorSchemeService {
  public readonly change$: Observable<PrefersColorScheme>;

  private prefersColorScheme: BehaviorSubject<PrefersColorScheme> = new BehaviorSubject(
    DEFAULT_COLORSCHEME
  );

  constructor(private userLocalStorageService: UserLocalStorageService) {
    this.prefersColorScheme.next(this.get());
    this.change$ = this.prefersColorScheme.asObservable();
  }

  get current(): PrefersColorScheme {
    return this.prefersColorScheme.value;
  }

  public get(): PrefersColorScheme {
    return (
      (this.userLocalStorageService.get('prefers-color-scheme') as PrefersColorScheme) ||
      this.detectSystemTheme()
    );
  }

  /**
   * Set the color scheme. The user preference will be stored in the browsers
   * local storage and will override the system theme.
   *
   * @returns Returns the color scheme that has been set.
   */
  public set(value: PrefersColorScheme): PrefersColorScheme {
    this.userLocalStorageService.set('prefers-color-scheme', value);
    this.prefersColorScheme.next(value);
    return this.current;
  }

  /**
   * Synchronize the internal state with the browsers local storage
   * or media query.
   */
  public sync(): void {
    const value: PrefersColorScheme = this.get();
    if (this.current !== value) {
      this.prefersColorScheme.next(value);
    }
  }

  /**
   * Toggles the current color scheme.
   *
   * @returns Returns the name of the now active color scheme.
   */
  public toggle(): PrefersColorScheme {
    return this.set(this.current === 'dark' ? 'light' : 'dark');
  }

  /**
   * Reset the color scheme to the default value. The user's preference won't
   * be affected by this.
   */
  public reset(): PrefersColorScheme {
    this.prefersColorScheme.next(DEFAULT_COLORSCHEME);
    return this.current;
  }

  private detectSystemTheme(): PrefersColorScheme {
    return window.matchMedia?.('(prefers-color-schema: dark)').matches ? 'dark' : 'light';
  }
}
