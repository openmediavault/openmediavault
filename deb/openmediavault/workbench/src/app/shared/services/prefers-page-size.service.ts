import { Injectable } from '@angular/core';
import { BehaviorSubject, Observable } from 'rxjs';

import { UserLocalStorageService } from '~/app/shared/services/user-local-storage.service';

@Injectable({
  providedIn: 'root'
})
export class PrefersPageSizeService {
  public readonly change$: Observable<number>;

  private readonly defaultPageSize: number = 25;

  private prefersPageSize: BehaviorSubject<number> = new BehaviorSubject(this.defaultPageSize);

  constructor(private userLocalStorageService: UserLocalStorageService) {
    this.prefersPageSize.next(this.get());
    this.change$ = this.prefersPageSize.asObservable();
  }

  get current(): number {
    return this.prefersPageSize.value;
  }

  public get(): number {
    return parseInt(this.userLocalStorageService.get('prefers-page-size', this.defaultPageSize));
  }

  /**
   * Set the color scheme.
   *
   * @returns Returns the color scheme that has been set.
   */
  public set(value: number): number {
    this.userLocalStorageService.set('prefers-page-size', String(value));
    this.prefersPageSize.next(value);
    return this.current;
  }

  /**
   * Synchronize the internal state with the browsers local storage
   * or media query.
   */
  public sync(): void {
    const value: number = this.get();
    if (this.current !== value) {
      this.prefersPageSize.next(value);
    }
  }
}
