import { Nl2brPipe } from '~/app/shared/pipes/nl2br.pipe';

describe('Nl2brPipe', () => {
  const pipe = new Nl2brPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transforms to nl2br (1)', () => {
    expect(pipe.transform('foo\nbar \n baz')).toBe('foo<br>\nbar <br>\n baz');
  });

  it('transforms to nl2br (2)', () => {
    expect(pipe.transform('foo \n baz')).toBe('foo <br>\n baz');
  });

  it('transforms to nl2br (3)', () => {
    expect(pipe.transform('foo \n baz')).toBe('foo <br>\n baz');
  });
});
