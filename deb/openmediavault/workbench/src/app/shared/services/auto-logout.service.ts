import { Injectable, NgZone, OnDestroy } from '@angular/core';
import { EMPTY, fromEvent, merge, Observable, Subject, timer } from 'rxjs';
import { filter, switchMap, takeUntil, tap, throttleTime } from 'rxjs/operators';

import { AuthService } from '~/app/shared/services/auth.service';
import { RpcRequestWithResponse, RpcService } from '~/app/shared/services/rpc.service';
import { UserLocalStorageService } from '~/app/shared/services/user-local-storage.service';

@Injectable({
  providedIn: 'root'
})
export class AutoLogoutService implements OnDestroy {
  private readonly activityEvents$: Observable<Event>;
  private readonly destroy$ = new Subject<void>();
  private timeout = 0;

  constructor(
    private authService: AuthService,
    private ngZone: NgZone,
    private rpcService: RpcService,
    private userLocalStorageService: UserLocalStorageService
  ) {
    this.activityEvents$ = merge(
      fromEvent(document, 'mousemove'),
      fromEvent(document, 'mousedown'),
      fromEvent(document, 'keydown'),
      fromEvent(document, 'wheel'),
      fromEvent(document, 'scroll', { passive: true }),
      fromEvent(document, 'touchstart', { passive: true }),
      fromEvent(document, 'touchmove', { passive: true }),
      fromEvent(document, 'touchend', { passive: true }),
      fromEvent(document, 'pointerdown'),
      fromEvent(document, 'pointermove')
    );
    // Watch for successful RPC requests to update the timeout setting
    // and to restart the auto-logout service.
    this.rpcService.request$
      .pipe(
        filter((req: RpcRequestWithResponse) => {
          return !req.error && req.service === 'WebGui' && req.method === 'setSettings';
        })
      )
      .subscribe((req: RpcRequestWithResponse) => {
        this.timeout = req.response.timeout * 60 * 1000;
        this.stop();
        this.start();
      });
  }

  ngOnDestroy(): void {
    this.destroy$.next();
    this.destroy$.complete();
  }

  start(): void {
    if (this.timeout <= 0) {
      return;
    }
    this.ngZone.runOutsideAngular(() => {
      const lastActivityKey = this.userLocalStorageService.buildKey('lastActivity');
      const crossTabActivity$ = fromEvent<StorageEvent>(window, 'storage').pipe(
        filter((e: StorageEvent) => e.key === lastActivityKey && e.newValue !== null)
      );
      const crossTabLogout$ = fromEvent<StorageEvent>(window, 'storage').pipe(
        filter((e: StorageEvent) => e.key === lastActivityKey && e.newValue === null),
        tap(() => this.logout())
      );
      const allActivity$ = merge(
        this.activityEvents$.pipe(
          throttleTime(500),
          tap(() => this.updateLastActivity())
        ),
        crossTabActivity$
      );

      allActivity$
        .pipe(
          switchMap(() => this.buildTimer()),
          takeUntil(this.destroy$)
        )
        .subscribe();
      crossTabLogout$.pipe(takeUntil(this.destroy$)).subscribe();

      this.buildTimer()
        .pipe(takeUntil(merge(allActivity$, this.destroy$)))
        .subscribe();
    });
  }

  stop(): void {
    this.destroy$.next();
  }

  load(): Observable<Record<string, any>> {
    return this.rpcService.request('WebGui', 'getSettings').pipe(
      tap((res: Record<string, any>) => {
        this.timeout = res.timeout * 60 * 1000;
      })
    );
  }

  private buildTimer(): Observable<never> {
    return timer(this.timeout).pipe(
      tap(() => this.logout()),
      switchMap(() => EMPTY)
    );
  }

  private updateLastActivity(): void {
    this.userLocalStorageService.set('lastActivity', Date.now().toString(), true);
  }

  private clearLastActivity(): void {
    this.userLocalStorageService.remove('lastActivity');
  }

  private logout(): void {
    this.stop();
    this.clearLastActivity();
    this.ngZone.run(() => {
      this.authService.logout().subscribe();
    });
  }
}
