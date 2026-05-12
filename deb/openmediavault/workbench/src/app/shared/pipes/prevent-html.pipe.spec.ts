import { PreventHtmlPipe } from '~/app/shared/pipes/prevent-html.pipe';

describe('PreventHtmlPipe', () => {
  const pipe = new PreventHtmlPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transform value (1)', () => {
    expect(pipe.transform('foo')).toBe('foo');
  });

  it('transform value (2)', () => {
    expect(pipe.transform(2)).toBe(2);
  });

  it('transform value (3)', () => {
    expect(pipe.transform('<foo>')).toBe('<foo>');
  });

  it('transform value (4)', () => {
    expect(pipe.transform('<span>foo</span>')).toBeNull();
  });
});
