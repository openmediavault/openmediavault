import { Injectable } from '@angular/core';
import { Title } from '@angular/platform-browser';
import { Subscription } from 'rxjs';
import { distinctUntilChanged, map } from 'rxjs/operators';

import { Unsubscribe } from '~/app/decorators';
import {
  SystemInformation,
  SystemInformationService
} from '~/app/shared/services/system-information.service';

const DEFAULT_TITLE = 'openmediavault Workbench';

@Injectable({
  providedIn: 'root'
})
export class TitleService {
  @Unsubscribe()
  private subscriptions = new Subscription();

  constructor(
    private systemInformationService: SystemInformationService,
    private title: Title
  ) {
    this.subscriptions.add(
      this.systemInformationService.systemInfo$
        .pipe(
          map((resp: SystemInformation): string => resp.hostname),
          distinctUntilChanged()
        )
        .subscribe((hostname: string) => {
          const newTitle = `${hostname} - ${DEFAULT_TITLE}`;
          this.title.setTitle(newTitle);
        })
    );
  }
}
