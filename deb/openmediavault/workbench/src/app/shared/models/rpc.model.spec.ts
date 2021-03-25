import { RpcObjectResponse } from '~/app/shared/models/rpc.model';

describe('RpcObjectResponse', () => {
  it('should transform', () => {
    const response: RpcObjectResponse = { foo: 'bar' };
    const result = RpcObjectResponse.transform(response, { foo: '{{ foo }} xyz', aaa: 'bbb' });
    expect(result).toEqual({ foo: 'bar xyz', aaa: 'bbb' });
  });

  it('should filter (mode=select)', () => {
    const response: RpcObjectResponse = { aaa: 'aaa', bbb: 'bbb', ccc: 'ccc' };
    const result = RpcObjectResponse.filter(response, ['aaa', 'bbb']);
    expect(result).toEqual({ aaa: 'aaa', bbb: 'bbb' });
  });

  it('should filter (mode=reject)', () => {
    const response: RpcObjectResponse = { aaa: 'aaa', bbb: 'bbb', ccc: 'ccc' };
    const result = RpcObjectResponse.filter(response, ['aaa', 'bbb'], 'omit');
    expect(result).toEqual({ ccc: 'ccc' });
  });
});
