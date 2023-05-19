import { TestBed } from '@angular/core/testing';
import * as _ from 'lodash';

import { DataStore } from '~/app/shared/models/data-store.type';
import { DataStoreService } from '~/app/shared/services/data-store.service';
import { TestingModule } from '~/app/testing.module';

describe('DataStoreService', () => {
  let service: DataStoreService;

  beforeEach(() => {
    TestBed.configureTestingModule({
      imports: [TestingModule]
    });
    service = TestBed.inject(DataStoreService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });

  it('should load inline data (1)', (done) => {
    const store: DataStore = {
      data: ['foo', 'bar'],
      fields: ['a', 'b']
    };
    service.load(store).subscribe((res) => {
      expect(store.data).toEqual([
        { a: 'foo', b: 'foo' },
        { a: 'bar', b: 'bar' }
      ]);
      expect(_.has(res, 'data')).toBeTruthy();
      expect(_.has(res, 'total')).toBeTruthy();
      done();
    });
  });

  it('should load inline data (2)', (done) => {
    const store: DataStore = {
      data: [
        ['password', 'Password'],
        ['pubkey', 'Public key']
      ],
      fields: ['a', 'b']
    };
    service.load(store).subscribe(() => {
      expect(store.data).toEqual([
        { a: 'password', b: 'Password' },
        { a: 'pubkey', b: 'Public key' }
      ]);
      done();
    });
  });

  it('should load inline data (3)', (done) => {
    const store: DataStore = {
      data: [
        [2, 3, 4],
        [7, 8]
      ],
      fields: ['a', 'b', 'c']
    };
    service.load(store).subscribe(() => {
      expect(store.data).toEqual([
        { a: 2, b: 3, c: 4 },
        { a: 7, b: 8, c: undefined }
      ]);
      done();
    });
  });

  it('should load inline data (4)', (done) => {
    const store: DataStore = {
      data: [
        { a: 'bar', b: 'bar' },
        { a: 'foo', b: 'foo' }
      ]
    };
    service.load(store).subscribe(() => {
      expect(store.data).toEqual([
        { a: 'bar', b: 'bar' },
        { a: 'foo', b: 'foo' }
      ]);
      done();
    });
  });

  it('should load inline data (5)', (done) => {
    const store: DataStore = {
      data: { a: 'aaa', b: 'bbb', c: 'ccc', d: 1 },
      fields: ['x', 'y']
    };
    service.load(store).subscribe(() => {
      expect(store.data).toEqual([
        { x: 'a', y: 'aaa' },
        { x: 'b', y: 'bbb' },
        { x: 'c', y: 'ccc' },
        { x: 'd', y: 1 }
      ]);
      done();
    });
  });

  it('should load inline data (6)', (done) => {
    const store: DataStore = {
      data: { a: 'aaa', b: 'bbb', c: 'ccc', d: 1 }
    };
    service.load(store).subscribe(() => {
      expect(store.data).toEqual([
        { key: 'a', value: 'aaa' },
        { key: 'b', value: 'bbb' },
        { key: 'c', value: 'ccc' },
        { key: 'd', value: 1 }
      ]);
      done();
    });
  });

  it('should load inline data (7)', (done) => {
    const store: DataStore = {
      data: [{ foo: 'a1' }, { foo: 'a2' }, { foo: 'a2' }, { foo: 'a3' }],
      uniqBy: 'foo'
    };
    service.load(store).subscribe(() => {
      expect(store.data).toEqual([{ foo: 'a1' }, { foo: 'a2' }, { foo: 'a3' }]);
      expect(store.data.length).toEqual(3);
      done();
    });
  });

  it('should assign additional sources', (done) => {
    const store: DataStore = {
      data: ['poweroff', 'hybrid', 'suspendhybrid'],
      fields: ['key', 'value'],
      assign: {
        key: 'key',
        sources: {
          poweroff: { value: 'Poweroff' },
          suspendhybrid: { value: 'Hybrid sleep' },
          hybrid: { text: 'foo bar baz' }
        }
      }
    };
    service.load(store).subscribe(() => {
      expect(store.data).toEqual([
        { key: 'poweroff', value: 'Poweroff' },
        { key: 'hybrid', text: 'foo bar baz', value: 'hybrid' },
        { key: 'suspendhybrid', value: 'Hybrid sleep' }
      ]);
      done();
    });
  });
});
