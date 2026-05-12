import { CountPipe } from '~/app/shared/pipes/count.pipe';

describe('CountPipe', () => {
  const pipe = new CountPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transforms to count (1)', () => {
    expect(pipe.transform('foo')).toEqual(3);
  });

  it('transforms to count (2)', () => {
    expect(pipe.transform({ foo: 11, bar: 22 })).toEqual(2);
  });

  it('transforms to count (3)', () => {
    expect(pipe.transform([1, 2, 3, 'bar'])).toEqual(4);
  });
});
