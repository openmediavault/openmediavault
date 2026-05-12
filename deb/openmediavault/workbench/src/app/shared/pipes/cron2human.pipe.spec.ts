import { Cron2humanPipe } from '~/app/shared/pipes/cron2human.pipe';

describe('Cron2humanPipe', () => {
  const pipe = new Cron2humanPipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transforms cron expression (1)', () => {
    expect(pipe.transform('* * * * *')).toBe('Every minute');
  });

  it('transforms cron expression (2)', () => {
    expect(pipe.transform('29 13 * * *')).toBe('At 01:29 PM');
  });

  it('transforms cron expression (3)', () => {
    expect(pipe.transform('*/5 13 * * 5')).toBe(
      'Every 5 minutes, between 01:00 PM and 01:59 PM, only on Friday'
    );
  });
});
