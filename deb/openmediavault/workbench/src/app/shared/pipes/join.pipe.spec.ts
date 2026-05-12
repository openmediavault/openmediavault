import { JoinPipe } from '~/app/shared/pipes/join.pipe';

describe('JoinPipe', () => {
  const pipe = new JoinPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transforms array (1)', () => {
    expect(pipe.transform(['a', 'b', 'c'])).toBe('a, b, c');
  });

  it('transforms array (2)', () => {
    expect(pipe.transform([1, 2, 3], '|')).toBe('1|2|3');
  });
});
