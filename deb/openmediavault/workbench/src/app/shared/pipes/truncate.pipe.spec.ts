import { TruncatePipe } from '~/app/shared/pipes/truncate.pipe';

describe('TruncatePipe', () => {
  const text = 'fsdfdsfs asdasd ador, sdaddfsfsd asdrz fdgthtzuz.';
  const pipe = new TruncatePipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('should truncate string (1)', () => {
    expect(pipe.transform(text, 27)).toEqual('fsdfdsfs asdasd ador, sd...');
  });

  it('should truncate string (2)', () => {
    expect(pipe.transform(text, 36, undefined, ' ')).toEqual('fsdfdsfs asdasd ador, sdaddfsfsd...');
  });

  it('should truncate string (3)', () => {
    expect(pipe.transform(text, 24, '', /,? +/)).toEqual('fsdfdsfs asdasd ador');
  });

  it('should not truncate string', () => {
    // @ts-ignore
    expect(pipe.transform(123456, 5)).toBe(123456);
  });
});
