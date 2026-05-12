import { BinaryUnitPipe } from '~/app/shared/pipes/binary-unit.pipe';

describe('BinaryUnitPipe', () => {
  const pipe = new BinaryUnitPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transforms to MiB', () => {
    expect(pipe.transform(2097152)).toBe('2.00 MiB');
  });

  it('transforms to KiB', () => {
    expect(pipe.transform(2097664, 2, 'KiB')).toBe('2048.50 KiB');
  });

  it('transforms to KiB (no decimal places)', () => {
    expect(pipe.transform(4096, 0)).toBe('4 KiB');
  });

  it('transforms to KiB (3 decimal places)', () => {
    expect(pipe.transform(2048, 3)).toBe('2.000 KiB');
  });
});
