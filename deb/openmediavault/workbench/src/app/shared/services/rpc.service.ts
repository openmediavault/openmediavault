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
import { HttpClient } from '@angular/common/http';
import { Injectable, NgZone } from '@angular/core';
import * as _ from 'lodash';
import { interval, Observable } from 'rxjs';
import { concatMap, map, mergeMap, takeWhile } from 'rxjs/operators';

import { retryDelayed, takeWhen } from '~/app/rxjs.helper';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { NotificationService } from '~/app/shared/services/notification.service';

type RpcResponse = {
  error?: any;
  response?: any;
};

export type RpcBgResponse = {
  filename: string;
  pos?: number;
  output?: string;
  outputPending?: boolean;
  running: boolean;
};

@Injectable({
  providedIn: 'root'
})
export class RpcService {
  private url = 'rpc.php';

  constructor(
    private http: HttpClient,
    private ngZone: NgZone,
    private notificationService: NotificationService
  ) {}

  /**
   * Execute the specified RPC.
   *
   * @param rpcService The name/class of the service to be executed.
   * @param rpcMethod The method name to be executed.
   * @param rpcParams The parameters of the method.
   * @param rpcOptions Optional RPC options.
   * @param maxRetries Number of retry attempts before failing.
   */
  request(
    rpcService: string,
    rpcMethod: string,
    rpcParams?: any,
    rpcOptions?: any,
    maxRetries?: number
  ): Observable<any> {
    const body: Record<string, any> = {
      service: rpcService,
      method: rpcMethod
    };
    if (!(_.isUndefined(rpcParams) || _.isNull(rpcParams))) {
      body.params = rpcParams;
    }
    if (!(_.isUndefined(rpcParams) || _.isNull(rpcParams))) {
      body.options = rpcOptions;
    }
    return this.http.post(this.url, body).pipe(
      retryDelayed(maxRetries),
      map((res: RpcResponse) => res.response)
    );
  }

  /**
   * Execute the specified task. The subscription will complete when
   * the task has been finished.
   *
   * @param rpcService The name/class of the service to be executed.
   * @param rpcMethod The method name to be executed.
   * @param rpcParams The parameters of the method.
   * @param rpcOptions Optional RPC options.
   * @param period The poll interval, defaults to 500 milliseconds.
   * @param maxRetries Number of retry attempts before failing.
   */
  requestTask(
    rpcService: string,
    rpcMethod: string,
    rpcParams?: any,
    rpcOptions?: any,
    period?: number,
    maxRetries?: number
  ): Observable<string> {
    return this.request(rpcService, rpcMethod, rpcParams, rpcOptions, maxRetries).pipe(
      mergeMap((filename: any) => this.pollTask(filename, period)),
      map((res: RpcBgResponse): string => {
        try {
          return JSON.parse(res.output);
        } catch (e) {}
        return res.output;
      })
    );
  }

  /**
   * Execute the specified background task. The subscription will complete
   * when the background process has been finished. The RPC response will
   * be forwarded to the subscriber.
   *
   * @param rpcService The name/class of the service to be executed.
   * @param rpcMethod The method name to be executed.
   * @param rpcParams The parameters of the method.
   * @param rpcOptions Optional RPC options.
   * @param pollPeriod The poll interval in milliseconds.
   * @param maxRetries Number of retry attempts before failing.
   */
  requestTaskOutput(
    rpcService: string,
    rpcMethod: string,
    rpcParams?: any,
    rpcOptions?: any,
    pollPeriod?: number,
    maxRetries?: number
  ): Observable<RpcBgResponse> {
    return this.request(rpcService, rpcMethod, rpcParams, rpcOptions, maxRetries).pipe(
      mergeMap((filename: any) => this.pollTaskOutput(filename, pollPeriod))
    );
  }

  /**
   * Stop the given task.
   *
   * @param filename The name of the background process status file.
   * @param maxRetries Number of retry attempts before failing.
   */
  stopTask(filename: string, maxRetries?: number): Observable<any> {
    return this.request(
      'Exec',
      'stop',
      {
        filename
      },
      undefined,
      maxRetries
    );
  }

  /**
   * Call the specified RPC that will trigger a file download.
   *
   * @param rpcService The name/class of the service to be executed.
   * @param rpcMethod The method name to be executed.
   * @param rpcParams The parameters of the method.
   */
  download(rpcService: string, rpcMethod: string, rpcParams: any) {
    this.ngZone.runOutsideAngular(() => {
      const body: Record<string, any> = {
        service: rpcService,
        method: rpcMethod
      };
      if (!(_.isUndefined(rpcParams) || _.isNull(rpcParams))) {
        body.params = rpcParams;
      }
      const request = new XMLHttpRequest();
      request.open('POST', 'download.php', true);
      request.responseType = 'blob';
      request.onreadystatechange = () => {
        if (request.readyState === 4 && request.status === 200) {
          const disposition = request.getResponseHeader('content-disposition');
          const type = request.getResponseHeader('Content-Type');
          const matches = /filename=(.*)/.exec(disposition);
          const filename = matches != null && matches[1] ? matches[1] : 'download.txt';
          const blob = new Blob([request.response], { type });
          const link = document.createElement('a');
          link.href = window.URL.createObjectURL(blob);
          link.download = filename;
          document.body.appendChild(link);
          link.click();
          document.body.removeChild(link);
        }
        if (request.readyState === 4 && request.status !== 200) {
          const reader = new FileReader();
          reader.onload = (e: any) => {
            const err = JSON.parse(e.target.result);
            const title = `${request.status} - ${request.statusText}`;
            const message = _.defaultTo(err.error.message, '');
            const traceback = _.defaultTo(err.error.traceback, '');
            this.notificationService.show(NotificationType.error, title, message, traceback);
          };
          reader.readAsText(request.response);
        }
      };
      request.send(JSON.stringify(body));
    });
  }

  /**
   * Poll the executed task until it has been finished.
   *
   * @param filename The name of the background process status file.
   * @param pollPeriod The poll interval in milliseconds. Defaults to
   *   500 milliseconds.
   * @param maxRetries Number of retry attempts before failing.
   * @return Returns an Observable containing the RPC response.
   */
  private pollTask(
    filename: string,
    pollPeriod?: number,
    maxRetries?: number
  ): Observable<RpcBgResponse> {
    pollPeriod = _.defaultTo(pollPeriod, 500);
    return interval(pollPeriod).pipe(
      concatMap(() =>
        this.request(
          'Exec',
          'getOutput',
          {
            filename,
            pos: 0
          },
          undefined,
          maxRetries
        )
      ),
      takeWhen((res: RpcBgResponse) => res.running === false)
    );
  }

  /**
   * Get the output of the given task.
   *
   * @param filename The name of the background process status file.
   * @param pollPeriod The poll interval in milliseconds. Defaults to
   *   500 milliseconds.
   * @param maxRetries Number of retry attempts before failing.
   */
  private pollTaskOutput(
    filename: string,
    pollPeriod?: number,
    maxRetries?: number
  ): Observable<RpcBgResponse> {
    let pos = 0;
    pollPeriod = _.defaultTo(pollPeriod, 500);
    return interval(pollPeriod).pipe(
      concatMap(() =>
        this.request(
          'Exec',
          'getOutput',
          {
            filename,
            pos,
            length: 1024 * 1024
          },
          undefined,
          maxRetries
        )
      ),
      takeWhile((res: RpcBgResponse) => {
        pos = res.pos;
        return res.running === true || res.outputPending;
      }, true)
    );
  }
}
