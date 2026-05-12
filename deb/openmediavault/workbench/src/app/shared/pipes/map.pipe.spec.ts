import { MapPipe } from '~/app/shared/pipes/map.pipe';

describe('MapPipe', () => {
  const pipe = new MapPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transforms to true [1/5]', () => {
    expect(pipe.transform('foo')).toBe('foo');
  });

  it('transforms to true [2/5]', () => {
    expect(pipe.transform('foo', { bar: 'BAR' })).toBe('foo');
  });

  it('transforms to true [3/5]', () => {
    expect(pipe.transform('five', { five: 5 })).toBe(5);
  });

  it('transforms to true [4/5]', () => {
    // eslint-disable-next-line @typescript-eslint/naming-convention
    expect(pipe.transform(5, { 5: 'Five' })).toBe('Five');
  });

  it('transforms to true [5/5]', () => {
    expect(pipe.transform(true, { true: 'TRUE' })).toBe('TRUE');
  });
});
