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
