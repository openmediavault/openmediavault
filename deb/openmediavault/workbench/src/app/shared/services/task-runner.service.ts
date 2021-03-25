import { Injectable } from '@angular/core';
import { BlockUI, NgBlockUI } from 'ng-block-ui';
import { Observable } from 'rxjs';
import { finalize } from 'rxjs/operators';

import { translate } from '~/app/i18n.helper';
import { RpcService } from '~/app/shared/services/rpc.service';

@Injectable({
  providedIn: 'root'
})
export class TaskRunnerService {
  @BlockUI()
  blockUI: NgBlockUI;

  constructor(private rpcService: RpcService) {}

  /**
   * Run a task and block the UI in the meanwhile.
   *
   * @param message The message that is displayed while the task
   *   is running.
   * @param rpcService The name/class of the service to be executed.
   * @param rpcMethod The method name to be executed.
   * @param rpcParams The parameters of the method.
   * @param rpcOptions Optional RPC options.
   * @param interval The poll interval.
   */
  execute(
    message: string,
    rpcService: string,
    rpcMethod: string,
    rpcParams?: any,
    rpcOptions?: any,
    interval?: number
  ): Observable<any> {
    this.blockUI.start(translate(message));
    return this.rpcService.requestTask(rpcService, rpcMethod, rpcParams, rpcOptions, interval).pipe(
      finalize(() => {
        this.blockUI.stop();
      })
    );
  }
}
