/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
import {
  Component,
  ElementRef,
  EventEmitter,
  Inject,
  OnDestroy,
  OnInit,
  Output,
  ViewChild,
  ViewEncapsulation
} from '@angular/core';
import { MAT_DIALOG_DATA } from '@angular/material/dialog';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { Subscription } from 'rxjs';
import stripAnsi from 'strip-ansi';

import { format } from '~/app/functions.helper';
import { Icon } from '~/app/shared/enum/icon.enum';
import { TaskDialogSshConfig } from '~/app/shared/models/task-dialog-ssh-config.type';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { Terminal } from "@xterm/xterm";
import { FitAddon } from "@xterm/addon-fit";
import { WebLinksAddon } from '@xterm/addon-web-links';
import { io, Socket } from 'socket.io-client';

/**
 * A dialog with 'Start', 'Stop' and 'Cancel' buttons. It will
 * display the content received by a background task.
 */
@Component({
  selector: 'omv-task-dialog-ssh',
  templateUrl: './task-dialog-ssh.component.html',
  styleUrls: ['./task-dialog-ssh.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class TaskDialogSshComponent implements OnInit, OnDestroy {
  @ViewChild('terminal', { static: true }) terminalDiv!: ElementRef;
  private socket: Socket;
  private term: Terminal;
  private inputBuffer: string = '';

  @Output()
  readonly finishEvent = new EventEmitter<string>();

  @ViewChild('content', { static: true })
  private content: ElementRef;
  container: HTMLElement;

  // Internal
  public config: TaskDialogSshConfig = {};
  public running = false;
  private subscription: Subscription;

  constructor(
    private authSessionService: AuthSessionService,
    @Inject(MAT_DIALOG_DATA) data: TaskDialogSshConfig
  ) {
    this.config = data;
    this.sanitizeConfig();
    this.socket = io('http://127.0.0.1:1122', { }); // Connect to server Socket.IO    
  }

  ngOnInit(): void {
    this.initializeTerminal();
    this.connectToWebSocket();

    if (this.config.startOnInit) {
      this.onStart();
    } else {
      this.print(
        format(
          'REQUEST ACCESS TO MASTER CONTROL PROGRAM.<br>' +
            'USER CODE 00-{{ username | upper }}.<br>' +
            'PASSWORD:MASTER. <br>' +
            '<span class="omv-text-blink">█</span>',
          {
            username: this.authSessionService.getUsername()
          }
        )
      );
    }
  }

  private initializeTerminal(): void {
    this.term = new Terminal();
    const fitAddon = new FitAddon();
    const webLinksAddon = new WebLinksAddon();

    this.term.loadAddon(fitAddon);
    this.term.loadAddon(webLinksAddon);

    this.term.open(document.getElementById('terminal'));
    fitAddon.fit();

    this.term.onData((data) => {
      if (data === '\r') {
        this.sendInputToServer();
      } else {
        this.inputBuffer += data;
        this.term.write(data);
      }
    });
  }

  private connectToWebSocket(): void {
    this.socket.on('connect', () => {
      this.term.write('Connected To SSH\r\n');
    });

    this.socket.on('output', (data: string) => {
      this.term.write(data);
    });

    this.socket.on('disconnect', () => {
      this.term.write('\r\nSSH server connection closed\r\n');
    });
  }

  private sendInputToServer(): void {
    if (this.socket && this.socket.connected) {
      this.socket.emit('input', this.inputBuffer);
      this.inputBuffer = '';
    }
  }

  ngOnDestroy(): void {
    if (this.socket) {
      this.socket.disconnect();
    }
  }

  onStart() {
    this.config.buttons.start.disabled = true;
    this.config.buttons.stop.disabled = false;
    this.config.buttons.close.disabled = false;
    this.config.buttons.close.dialogResult = false;
    this.content.nativeElement.innerHTML = '';
    this.running = true;
  }

  onStop(): void {
    this.subscription?.unsubscribe();
  }

  protected sanitizeConfig(): void {
    this.config.icon = _.get(Icon, this.config.icon, this.config.icon);
    _.defaultsDeep(this.config, {
      autoScroll: true,
      startOnInit: false,
      showCompletion: true,
      buttons: {
        start: {
          text: gettext('Start'),
          hidden: false,
          disabled: false,
          autofocus: false,
          class: 'bg-primary'
        },
        stop: {
          text: gettext('Stop'),
          hidden: false,
          disabled: true,
          autofocus: false,
          class: 'bg-warning'
        },
        close: {
          text: gettext('Close'),
          hidden: false,
          disabled: false,
          autofocus: false,
          class: 'bg-indigo',
          dialogResult: false
        }
      }
    });
    if (this.config.startOnInit) {
      this.config.buttons.start.hidden = true;
    }
  }

  private print(text: string, escape: boolean = false): void {
    const nativeEl = this.content.nativeElement;
    const doScroll: boolean =
      nativeEl.scrollHeight - nativeEl.clientHeight <= nativeEl.scrollTop + 1;
    // Make sure we do not exceed a max. size of displayed
    // content to keep the memory consumption low.
    // Allow displaying up to 100.000 lines (80 char per line) of
    // content, then reduce to 75.000 lines in one step (smaller
    // steps may be repeated to often which is too costly).
    // Note, if your task is crossing this border, please keep in
    // mind that this dialog is not meant to be used for such
    // excessive tasks.
    if (nativeEl.innerHTML.length > 100000 * 80) {
      // Search for the first line break after the specified
      // position and cut it there if found.
      const startSearch = nativeEl.innerHTML.length - 75000 * 80;
      const pos = nativeEl.innerHTML.indexOf('\n', startSearch);
      nativeEl.innerHTML = nativeEl.innerHTML.slice(pos > 0 ? pos : startSearch);
    }
    // Strip ASCII escape codes and escape characters to
    // HTML-safe sequences if necessary.
    nativeEl.innerHTML += stripAnsi(escape ? format('{{ text | escape }}', { text }) : text);
    if (this.config.autoScroll && _.isFunction(nativeEl.scroll) && doScroll) {
      nativeEl.scroll({ behavior: 'auto', top: nativeEl.scrollHeight });
    }
  }
}
