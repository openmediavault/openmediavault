import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import * as _ from 'lodash';
import { ToastrModule } from 'ngx-toastr';
import { of, Subject, throwError } from 'rxjs';

import { DatatablePageComponent } from '~/app/core/components/intuition/datatable-page/datatable-page.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { DataStoreService } from '~/app/shared/services/data-store.service';
import { RpcService } from '~/app/shared/services/rpc.service';
import { TestingModule } from '~/app/testing.module';

describe('DatatablePageComponent', () => {
  let component: DatatablePageComponent;
  let fixture: ComponentFixture<DatatablePageComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [IntuitionModule, TestingModule, ToastrModule.forRoot()]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DatatablePageComponent);
    component = fixture.componentInstance;
    component.config = {
      columns: [],
      store: {
        data: []
      }
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  describe('data loading', () => {
    let request: jest.SpyInstance;

    beforeEach(() => {
      request = jest
        .spyOn(TestBed.inject(RpcService), 'request')
        .mockReturnValue(of([{ name: 'a' }]));
      component.config = {
        columns: [],
        store: {
          proxy: {
            service: 'Foo',
            get: { method: 'get{{ name }}', params: { id: '{{ id }}' } }
          },
          transform: { text: 'x-{{ name }}' }
        }
      };
      component.pageContextService.set({ name: 'ctx', id: '1' });
    });

    it('should format the request properties and keep the configuration', () => {
      component.onLoadDataEvent({ offset: 1, limit: 10, dir: 'asc', prop: 'name', search: 'x' });
      expect(request).toHaveBeenLastCalledWith('Foo', 'getctx', {
        id: '1',
        start: 10,
        limit: 10,
        sortdir: 'asc',
        sortfield: 'name',
        search: 'x'
      });
      // The tokens must be kept in the configuration.
      expect(component.config.store.proxy.get.method).toBe('get{{ name }}');
      expect(component.config.store.proxy.get.params).toEqual({ id: '{{ id }}' });
    });

    it('should not keep the paging, sorting and searching parameters', () => {
      component.onLoadDataEvent({ offset: 1, limit: 10, dir: 'asc', prop: 'name', search: 'x' });
      component.onLoadDataEvent({});
      // The second request must not contain the parameters of the first one.
      expect(request).toHaveBeenLastCalledWith('Foo', 'getctx', {
        id: '1',
        start: 0,
        limit: -1
      });
    });

    it('should update the data of the configured store', () => {
      component.onLoadDataEvent({});
      // The transform must use the loaded data and not the page context.
      expect(component.config.store.data).toEqual([{ name: 'a', text: 'x-a' }]);
      // The service sets the default fields if they are not configured.
      expect(component.config.store.fields).toEqual(['key', 'value']);
      expect(component.config.store.transform).toEqual({ text: 'x-{{ name }}' });
    });
  });

  describe('tree mode', () => {
    let dataStoreService: DataStoreService;
    let load: jest.SpyInstance;
    let omitTreeStatus = false;

    const rows = () => component.config.store.data;

    beforeEach(() => {
      omitTreeStatus = false;
      fixture = TestBed.createComponent(DatatablePageComponent);
      component = fixture.componentInstance;
      component.config = {
        columns: [],
        treeFromRelation: 'parent',
        treeToRelation: 'path',
        store: {
          proxy: {
            service: 'Foo',
            get: {
              method: 'getList',
              params: {
                path: "{{ _parent.path | default('/') | safe }}"
              }
            }
          }
        }
      };
      dataStoreService = TestBed.inject(DataStoreService);
      load = jest.spyOn(dataStoreService, 'load').mockImplementation((store) => {
        const path = store.proxy.get.params.path;
        const data = _.get(
          {
            root: [{ path: '/a', treeStatus: 'collapsed' }],
            a: [
              { path: '/a/b', treeStatus: 'collapsed' },
              { path: '/a/c', treeStatus: 'disabled' }
            ],
            b: []
          },
          _.last(_.split(path, '/')) || 'root'
        );
        const responseData = _.map(data, (row) =>
          omitTreeStatus ? _.omit(row, 'treeStatus') : row
        );
        store.data = responseData;
        return of({ data: responseData, total: responseData.length });
      });
      fixture.detectChanges();
      // Load the root rows, the datatable does this asynchronously.
      component.onLoadDataEvent({});
    });

    it('should load the child rows of an expanded row', () => {
      component.onTreeAction(rows()[0]);
      expect(load).toHaveBeenLastCalledWith(
        expect.objectContaining({
          proxy: expect.objectContaining({
            get: expect.objectContaining({ params: { path: '/a', start: 0, limit: -1 } })
          })
        })
      );
      expect(rows()[0].treeStatus).toBe('expanded');
      expect(rows().slice(1)).toEqual([
        { path: '/a/b', treeStatus: 'collapsed', parent: '/a' },
        { path: '/a/c', treeStatus: 'disabled', parent: '/a' }
      ]);
    });

    it('should format the filters of the child request and keep the configuration', () => {
      component.config.store.filters = [
        { operator: 'ne', arg0: { prop: 'type' }, arg1: '{{ name }}' }
      ];
      component.pageContextService.set({ name: 'dir' });
      component.onTreeAction(rows()[0]);
      expect(load).toHaveBeenLastCalledWith(
        expect.objectContaining({
          filters: [{ operator: 'ne', arg0: { prop: 'type' }, arg1: 'dir' }]
        })
      );
      // The tokens must be kept in the configuration.
      expect(component.config.store.filters[0].arg1).toBe('{{ name }}');
    });

    it('should disable remote paging', () => {
      fixture = TestBed.createComponent(DatatablePageComponent);
      component = fixture.componentInstance;
      component.config = {
        columns: [],
        treeFromRelation: 'parent',
        treeToRelation: 'path',
        remotePaging: true,
        store: { data: [] }
      };
      fixture.detectChanges();

      expect(component.config.remotePaging).toBe(false);
    });

    it('should not load the child rows twice', () => {
      component.onTreeAction(rows()[0]);
      component.onTreeAction(rows()[0]);
      expect(rows()[0].treeStatus).toBe('collapsed');
      component.onTreeAction(rows()[0]);
      expect(rows()[0].treeStatus).toBe('expanded');
      expect(rows().length).toBe(3);
      expect(load).toHaveBeenCalledTimes(2);
    });

    it('should disable a row without child rows', () => {
      component.onTreeAction(rows()[0]);
      component.onTreeAction(rows()[1]);
      expect(rows()[1].treeStatus).toBe('disabled');
    });

    it('should not add rows that already exist', () => {
      component.onTreeAction(rows()[0]);
      load.mockReturnValue(of({ data: [{ path: '/a/b' }], total: 1 }));
      component.onTreeAction(rows()[1]);
      expect(rows().length).toBe(3);
      expect(rows()[1].treeStatus).toBe('disabled');
    });

    it('should expand the rows again after reload', () => {
      component.onTreeAction(rows()[0]);
      component.onLoadDataEvent({});
      expect(rows().map((row) => [row.path, row.treeStatus])).toEqual([
        ['/a', 'expanded'],
        ['/a/b', 'collapsed'],
        ['/a/c', 'disabled']
      ]);
    });

    it('should handle a missing treeStatus like collapsed', () => {
      delete rows()[0].treeStatus;
      component.onTreeAction(rows()[0]);
      expect(rows()[0].treeStatus).toBe('expanded');
    });

    it('should collapse the row on error and not re-expand on reload', () => {
      load.mockReturnValueOnce(throwError(() => new Error('failed')));
      component.onTreeAction(rows()[0]);
      expect(rows()[0].treeStatus).toBe('collapsed');
      component.onLoadDataEvent({});
      expect(rows()[0].treeStatus).toBe('collapsed');
    });

    it('should not request the child rows of a row without children again after reload', () => {
      component.onTreeAction(rows()[0]);
      component.onTreeAction(rows()[1]);
      expect(rows()[1].treeStatus).toBe('disabled');
      load.mockClear();
      component.onLoadDataEvent({});
      // Only the root rows and the child rows of '/a' are requested.
      expect(load).toHaveBeenCalledTimes(2);
      expect(rows()[1].treeStatus).toBe('collapsed');
    });

    it('should expand a row without treeStatus again after reload', () => {
      omitTreeStatus = true;
      component.onLoadDataEvent({});
      component.onTreeAction(rows()[0]);
      expect(rows()[0].treeStatus).toBe('expanded');
      expect(rows().length).toBe(3);
      component.onLoadDataEvent({});
      expect(rows()[0].treeStatus).toBe('expanded');
      expect(rows().length).toBe(3);
    });

    it('should ignore the error of a request for a row that has been reloaded', () => {
      const pending = new Subject<any>();
      load.mockReturnValueOnce(pending);
      component.onTreeAction(rows()[0]);
      expect(rows()[0].treeStatus).toBe('loading');
      // The reload replaces the row and expands it again.
      component.onLoadDataEvent({});
      expect(rows()[0].treeStatus).toBe('expanded');
      pending.error(new Error('failed'));
      expect(rows()[0].treeStatus).toBe('expanded');
      // The row must still be expanded after another reload.
      component.onLoadDataEvent({});
      expect(rows()[0].treeStatus).toBe('expanded');
    });

    it('should not disable a row whose child rows have been added by another response', () => {
      component.config.store.data = [
        { path: '/a', treeStatus: 'collapsed' },
        { path: '/b', treeStatus: 'collapsed' }
      ];
      const tree = () => ({
        data: [
          { path: '/a/x', parent: '/a', treeStatus: 'disabled' },
          { path: '/b/y', parent: '/b', treeStatus: 'disabled' }
        ],
        total: 2
      });
      const pendingA = new Subject<any>();
      const pendingB = new Subject<any>();
      load.mockReturnValueOnce(pendingA).mockReturnValueOnce(pendingB);
      component.onTreeAction(rows()[0]);
      component.onTreeAction(rows()[1]);
      pendingA.next(tree());
      pendingB.next(tree());
      expect(rows()[0].treeStatus).toBe('expanded');
      expect(rows()[1].treeStatus).toBe('expanded');
      expect(rows().length).toBe(4);
    });

    it('should use the parameters of the response that provided the rows for the child rows', () => {
      // Expand the row, so it is expanded again after the reload.
      component.onTreeAction(rows()[0]);
      const pendingA = new Subject<any>();
      const pendingB = new Subject<any>();
      load.mockReturnValueOnce(pendingA).mockReturnValueOnce(pendingB);
      component.onLoadDataEvent({ dir: 'asc', prop: 'path' });
      component.onLoadDataEvent({ dir: 'desc', prop: 'path' });
      // The response of the first request arrives after the second has been started.
      pendingA.next({ data: [{ path: '/a', treeStatus: 'collapsed' }], total: 1 });
      expect(load).toHaveBeenLastCalledWith(
        expect.objectContaining({
          proxy: expect.objectContaining({
            get: expect.objectContaining({
              params: expect.objectContaining({ path: '/a', sortdir: 'asc', sortfield: 'path' })
            })
          })
        })
      );
    });

    it('should use the sorting and searching of the last request for the child rows', () => {
      component.onLoadDataEvent({ dir: 'desc', prop: 'path', search: 'foo', offset: 1, limit: 10 });
      component.onTreeAction(rows()[0]);
      expect(load).toHaveBeenLastCalledWith(
        expect.objectContaining({
          proxy: expect.objectContaining({
            get: expect.objectContaining({
              params: {
                path: '/a',
                start: 0,
                limit: -1,
                sortdir: 'desc',
                sortfield: 'path',
                search: 'foo'
              }
            })
          })
        })
      );
    });
  });
});
