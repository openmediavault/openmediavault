import dayjs from 'dayjs';

import { LocaleDatePipe } from '~/app/shared/pipes/locale-date.pipe';

describe('LocaleDatePipe', () => {
  const pipe = new LocaleDatePipe();

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('transform date into human readable time (1)', () => {
    const date: Date = dayjs().subtract(130, 'seconds').toDate();
    expect(pipe.transform(date, 'relative')).toBe('2 minutes ago');
  });

  it('transform date into human readable time (2)', () => {
    const date: string = dayjs().subtract(40, 'seconds').toISOString();
    expect(pipe.transform(date, 'relative')).toBe('a few seconds ago');
  });

  it('transform date into human readable time (3)', () => {
    const date: Date = dayjs().subtract(70, 'minutes').toDate();
    expect(pipe.transform(date, 'relative')).toBe('an hour ago');
  });

  it('transform date into human readable time (4)', () => {
    const date: number = dayjs().subtract(3, 'days').unix();
    expect(pipe.transform(date, 'relative')).toBe('3 days ago');
  });

  it('transform date into human readable time (5)', () => {
    const date: Date = dayjs('2013-02-04T22:44:30.652Z').toDate();
    expect(pipe.transform(date, 'time')).toBe('23:44:30');
  });

  it('transform date into human readable time (6)', () => {
    const date: Date = dayjs('2013-02-04T22:44:30.652Z').toDate();
    expect(pipe.transform(date, 'datetime')).toBe('2013-2-4 23:44:30');
  });

  it('transform date into human readable time (7)', () => {
    const date: Date = dayjs('2013-02-04T22:44:30.652Z').toDate();
    expect(pipe.transform(date, 'date')).toBe('2013-2-4');
  });

  it('transform date into human readable time (8)', () => {
    const date: Date = dayjs().subtract(70, 'minutes').toDate();
    expect(pipe.transform(date, 'relative', true)).toBe('an hour');
  });
});
