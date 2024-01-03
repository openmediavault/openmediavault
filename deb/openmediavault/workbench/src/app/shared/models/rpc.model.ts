/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2024 Volker Theile
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
import * as _ from 'lodash';

import { formatDeep } from '~/app/functions.helper';

export type RpcListResponse = {
  data: Array<any>;
  total: number;
};

export type RpcObjectResponse = Record<string, any>;

// eslint-disable-next-line @typescript-eslint/naming-convention
export const RpcObjectResponse = {
  transform: (response: RpcObjectResponse, config: Record<string, string>): RpcObjectResponse => {
    const tmp = formatDeep(config, response);
    _.merge(response, tmp);
    return response;
  },

  filter: (
    response: RpcObjectResponse,
    keys: string[],
    mode: 'pick' | 'omit' = 'pick'
  ): RpcObjectResponse => {
    switch (mode) {
      case 'pick':
        response = _.pick(response, keys);
        break;
      case 'omit':
        response = _.omit(response, keys);
        break;
    }
    return response;
  }
};
