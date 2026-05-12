import { MapIconEnumPipe } from '~/app/shared/pipes/map-icon-enum.pipe';

describe('MapIconEnumPipe', () => {
  const pipe = new MapIconEnumPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transforms to icon string [1]', () => {
    expect(pipe.transform('user')).toBe('mdi:account');
  });

  it('transforms to icon string [2]', () => {
    expect(pipe.transform('skipPrevious')).toBe('mdi:skip-previous');
  });

  it('transforms to icon string [3]', () => {
    expect(pipe.transform('mdi:arrow-up')).toBe('mdi:arrow-up');
  });

  it('transforms to icon string [4]', () => {
    expect(pipe.transform('foo')).toBe('foo');
  });

  it('transforms to icon string [5]', () => {
    expect(pipe.transform('foo', 'mdi:power')).toBe('mdi:power');
  });
});
