import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { ToastrModule } from 'ngx-toastr';
import { of } from 'rxjs';

import { DatatablePageComponent } from '~/app/core/components/intuition/datatable-page/datatable-page.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
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
});
