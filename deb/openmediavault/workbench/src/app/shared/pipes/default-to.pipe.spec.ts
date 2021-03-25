import { DefaultToPipe } from '~/app/shared/pipes/default-to.pipe';

describe('DefaultToPipe', () => {
  const pipe = new DefaultToPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transform value (1)', () => {
    const value = null;
    expect(pipe.transform(value)).toBe(undefined);
  });

  it('transform value (2)', () => {
    const value = undefined;
    expect(pipe.transform(value, 'n/a')).toBe('n/a');
  });

  it('transform value (3)', () => {
    const value = '';
    expect(pipe.transform(value, '--')).toBe('--');
  });

  it('transform value', () => {
    const value = 'foo';
    expect(pipe.transform(value)).toBe('foo');
  });
});
