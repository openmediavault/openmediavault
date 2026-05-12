import { UpperFirstPipe } from '~/app/shared/pipes/upper-first.pipe';

describe('UpperFirstPipe', () => {
  const pipe = new UpperFirstPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transform text (1)', () => {
    expect(pipe.transform('hello')).toBe('Hello');
  });

  it('transform text (2)', () => {
    expect(pipe.transform('Hello')).toBe('Hello');
  });

  it('transform text (3)', () => {
    expect(pipe.transform('hELlo')).toBe('HELlo');
  });
});
