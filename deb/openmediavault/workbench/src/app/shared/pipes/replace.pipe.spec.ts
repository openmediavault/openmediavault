import { ReplacePipe } from '~/app/shared/pipes/replace.pipe';

describe('ReplacePipe', () => {
  const pipe = new ReplacePipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transforms to replace (1)', () => {
    expect(pipe.transform('foo bar baz', 'bar', 'xyz')).toBe('foo xyz baz');
  });

  it('transforms to replace (2)', () => {
    expect(pipe.transform('a<br>b', '<br>', '\n')).toBe('a\nb');
  });
});
