import { EncodeUriComponentPipe } from '~/app/shared/pipes/encode-uri-component.pipe';

describe('EncodeUriComponentPipe', () => {
  const pipe = new EncodeUriComponentPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('should transform value (1)', () => {
    expect(pipe.transform('/dev/sda1')).toEqual(encodeURIComponent('/dev/sda1'));
  });

  it('should transform value (2)', () => {
    expect(pipe.transform(101)).toEqual('101');
  });

  it('should transform value (3)', () => {
    expect(pipe.transform(false)).toEqual('false');
  });
});
