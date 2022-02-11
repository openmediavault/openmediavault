import { Br2nlPipe } from '~/app/shared/pipes/br2nl.pipe';

describe('Br2nlPipe', () => {
  const pipe = new Br2nlPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transforms to br2nl (1)', () => {
    expect(pipe.transform('foo<br>bar <br> baz')).toBe('foo\nbar \n baz');
  });

  it('transforms to br2nl (2)', () => {
    expect(pipe.transform('foo <br/> baz')).toBe('foo \n baz');
  });

  it('transforms to br2nl (3)', () => {
    expect(pipe.transform('foo <br   /> baz')).toBe('foo \n baz');
  });
});
