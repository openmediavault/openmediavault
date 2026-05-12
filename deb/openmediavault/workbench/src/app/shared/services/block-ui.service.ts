import { Injectable } from '@angular/core';
import { BehaviorSubject, Observable } from 'rxjs';

type BlockUiConfig = {
  active: boolean;
  message?: string;
};

@Injectable({
  providedIn: 'root'
})
export class BlockUiService {
  public readonly config$: Observable<BlockUiConfig>;

  private config: BehaviorSubject<BlockUiConfig> = new BehaviorSubject<BlockUiConfig>({
    active: false,
    message: undefined
  });

  constructor() {
    this.config$ = this.config.asObservable();
  }

  start(message?: string): void {
    this.config.next({ active: true, message });
  }

  stop(): void {
    this.config.next({ ...this.config.value, active: false });
  }

  update(message?: string): void {
    this.config.next({ ...this.config.value, message });
  }

  resetGlobal(): void {
    this.config.next({ active: false, message: undefined });
  }
}
