import { TemplatePipe } from '~/app/shared/pipes/template.pipe';

describe('TemplatePipe', () => {
  const pipe = new TemplatePipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transforms (1)', () => {
    expect(pipe.transform('{{ a }}', { a: 'hello' })).toBe('hello');
  });
});
