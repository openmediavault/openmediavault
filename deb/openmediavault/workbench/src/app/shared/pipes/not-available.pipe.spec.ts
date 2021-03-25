import { NotAvailablePipe } from '~/app/shared/pipes/not-available.pipe';

describe('NotAvailablePipe', () => {
  const pipe = new NotAvailablePipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transforms with value (1)', () => {
    const value = null;
    expect(pipe.transform(value)).toBe('n/a');
  });

  it('transforms with value (2)', () => {
    const value = undefined;
    expect(pipe.transform(value)).toBe('n/a');
  });

  it('transforms with value (3)', () => {
    const value = '';
    expect(pipe.transform(value)).toBe('n/a');
  });

  it('transforms with some value', () => {
    const value = 'bar';
    expect(pipe.transform(value)).toBe('bar');
  });

  it('transforms with other text', () => {
    const value = undefined;
    expect(pipe.transform(value, 'Unknown')).toBe('Unknown');
  });
});
