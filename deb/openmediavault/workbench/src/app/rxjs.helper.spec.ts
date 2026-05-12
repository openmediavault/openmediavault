import { TestBed } from '@angular/core/testing';
import { from } from 'rxjs';
import { TestScheduler } from 'rxjs/testing';

import { retryDelayed, takeWhen } from '~/app/rxjs.helper';
import { RpcService } from '~/app/shared/services/rpc.service';
import { TestingModule } from '~/app/testing.module';

describe('rxjs.helper', () => {
  const testScheduler = new TestScheduler((actual, expected) => {
    expect(actual).toEqual(expected);
  });

  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [TestingModule]
    })
  );

  it('should be created', () => {
    const service: RpcService = TestBed.inject(RpcService);
    expect(service).toBeTruthy();
  });

  it('should test takeWhen operator [1]', (done) => {
    from([1, 2, 3])
      .pipe(takeWhen((value) => value % 3 === 0))
      .subscribe((result) => {
        expect(result).toBe(3);
        done();
      });
  });

  it('should test takeWhen operator [2]', (done) => {
    from(['a', 'b', 'c'])
      .pipe(takeWhen((value) => value === 'd'))
      .subscribe({
        next: () => {
          done.fail('The subscribers next() should not be called.');
        },
        complete: () => {
          done();
        }
      });
  });

  it('should test retryDelayed operator [1]', () => {
    testScheduler.run(({ expectObservable, cold }) => {
      const source = cold('---1--|');
      const result = source.pipe(retryDelayed());
      const expected = '---1--|';
      expectObservable(result).toBe(expected);
    });
  });

  it('should test retryDelayed operator [2]', () => {
    testScheduler.run(({ expectObservable, cold }) => {
      const source = cold('#');
      const result = source.pipe(retryDelayed(1, 10));
      const expected = '----------------#';
      expectObservable(result).toBe(expected);
    });
  });

  it('should test retryDelayed operator [3]', () => {
    testScheduler.run(({ expectObservable, cold }) => {
      const source = cold('#');
      const result = source.pipe(retryDelayed(2, 10));
      const expected = '------------------------------------#';
      expectObservable(result).toBe(expected);
    });
  });
});
