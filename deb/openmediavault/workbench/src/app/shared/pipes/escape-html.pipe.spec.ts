import { EscapeHtmlPipe } from '~/app/shared/pipes/escape-html.pipe';

describe('TrustHtmlPipe', () => {
  const pipe: EscapeHtmlPipe = new EscapeHtmlPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transform value (1)', () => {
    const value = null;
    expect(pipe.transform(value)).toBe(value);
  });

  it('transform value (2)', () => {
    const value = 1;
    expect(pipe.transform(value)).toBe(value);
  });

  it('transform value (3)', () => {
    const value = '<f..tp..... foo bar';
    expect(pipe.transform(value)).toBe('&lt;f..tp..... foo bar');
  });
});
