import { ArrayPipe } from '~/app/shared/pipes/array.pipe';

describe('ArrayPipe', () => {
  const pipe = new ArrayPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transforms string to array', () => {
    expect(pipe.transform('foo')).toEqual(['foo']);
  });

  it('transforms number to array', () => {
    expect(pipe.transform(2)).toEqual([2]);
  });

  it('transforms array to array', () => {
    expect(pipe.transform(['bar'], true)).toEqual([['bar']]);
  });

  it('not transforms array to array', () => {
    expect(pipe.transform(['xyz'])).toEqual(['xyz']);
  });
});
