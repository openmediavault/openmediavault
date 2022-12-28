import { SplitPipe } from '~/app/shared/pipes/split.pipe';

describe('SplitPipe', () => {
  const pipe = new SplitPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transform value (1)', () => {
    expect(pipe.transform(100)).toEqual(100);
  });

  it('transform value (2)', () => {
    expect(pipe.transform('foo')).toEqual(['foo']);
  });

  it('transform value (3)', () => {
    expect(pipe.transform('foo,bar', ',')).toEqual(['foo', 'bar']);
  });

  it('transform value (4)', () => {
    expect(pipe.transform('')).toEqual([]);
  });
});
