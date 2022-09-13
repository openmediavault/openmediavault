import { Injectable } from '@angular/core';
import { BehaviorSubject, Observable } from 'rxjs';

export type PrefersColorScheme = 'light' | 'dark';

@Injectable({
  providedIn: 'root'
})
export class PrefersColorSchemeService {
  public readonly change$: Observable<PrefersColorScheme>;

  get current(): PrefersColorScheme {
    return this.prefersColorScheme.value;
  }

  private prefersColorScheme: BehaviorSubject<PrefersColorScheme> = new BehaviorSubject('light');

  constructor() {
    this.prefersColorScheme.next(this.get());
    this.change$ = this.prefersColorScheme.asObservable();
  }

  public get(): PrefersColorScheme {
    return (
      (localStorage.getItem('prefers-color-scheme') as PrefersColorScheme) ||
      this.detectSystemTheme()
    );
  }

  public set(value: PrefersColorScheme): PrefersColorScheme {
    localStorage.setItem('prefers-color-scheme', value);
    this.prefersColorScheme.next(value);
    return value;
  }

  public toggle(): PrefersColorScheme {
    return this.set(this.current === 'dark' ? 'light' : 'dark');
  }

  private detectSystemTheme(): PrefersColorScheme {
    return window.matchMedia?.('(prefers-color-schema: dark)').matches ? 'dark' : 'light';
  }
}
