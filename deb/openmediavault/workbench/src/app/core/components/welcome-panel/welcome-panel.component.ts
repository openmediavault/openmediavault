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
import { ChangeDetectionStrategy, Component } from '@angular/core';

import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  selector: 'omv-welcome-panel',
  templateUrl: './welcome-panel.component.html',
  styleUrls: ['./welcome-panel.component.scss'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class WelcomePanelComponent {
  constructor(private rpcService: RpcService) {}
  onClosed(): void {
    this.rpcService.request('WebGui', 'dismissWelcomeMessage').subscribe();
  }
}
