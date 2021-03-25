import { MaxPipe } from '~/app/shared/pipes/max.pipe';

describe('MaxPipe', () => {
  const pipe = new MaxPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transforms to min (1)', () => {
    expect(pipe.transform(-1, 0)).toBe(0);
  });

  it('transforms to min (2)', () => {
    expect(pipe.transform(10, 2)).toBe(10);
  });
});
