import { SortPipe } from '~/app/shared/pipes/sort.pipe';

describe('SortPipe', () => {
  const pipe = new SortPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transform value (1)', () => {
    expect(pipe.transform([3, 2, 1])).toEqual([1, 2, 3]);
  });

  it('transform value (2)', () => {
    expect(pipe.transform(['y', 'a', 'm'])).toEqual(['a', 'm', 'y']);
  });
});
