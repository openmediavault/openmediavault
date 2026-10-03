import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import * as _ from 'lodash';
import { ToastrModule } from 'ngx-toastr';
import { of } from 'rxjs';

import { DatatablePageComponent } from '~/app/core/components/intuition/datatable-page/datatable-page.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { DataStoreService } from '~/app/shared/services/data-store.service';
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

  describe('tree mode', () => {
    let dataStoreService: DataStoreService;
    let load: jest.SpyInstance;

    const rows = () => component.config.store.data;

    beforeEach(() => {
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
                path: "{{ _parent.path | default('/') }}"
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
        store.data = data;
        return of({ data, total: data.length });
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
  });
});
