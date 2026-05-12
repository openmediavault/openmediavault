import dayjs from 'dayjs';

import * as functions from '~/app/functions.helper';
import { LocaleDatePipe } from '~/app/shared/pipes/locale-date.pipe';

describe('LocaleDatePipe', () => {
  const pipe = new LocaleDatePipe();

  beforeEach(() => {
    jest.spyOn(functions, 'dateToLocale').mockImplementation((date: Date, format: string) => {
      let result;
      switch (format) {
        case 'datetime':
          result = date.toLocaleString('de-DE', { timeZone: 'Europe/Berlin' });
          break;
        case 'time':
          result = date.toLocaleTimeString('de-DE', { timeZone: 'Europe/Berlin' });
          break;
        case 'date':
          result = date.toLocaleDateString('de-DE', { timeZone: 'Europe/Berlin' });
          break;
      }
      return result;
    });
  });

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
    expect(pipe.transform(date, 'datetime')).toBe('4.2.2013, 23:44:30');
  });

  it('transform date into human readable time (7)', () => {
    const date: Date = dayjs('2013-02-04T22:44:30.652Z').toDate();
    expect(pipe.transform(date, 'date')).toBe('4.2.2013');
  });

  it('transform date into human readable time (8)', () => {
    const date: Date = dayjs().subtract(70, 'minutes').toDate();
    expect(pipe.transform(date, 'relative', true)).toBe('an hour');
  });
});
