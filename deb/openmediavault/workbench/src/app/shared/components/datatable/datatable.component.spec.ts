import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { ToastrModule } from 'ngx-toastr';

import { ComponentsModule } from '~/app/shared/components/components.module';
import { DatatableComponent } from '~/app/shared/components/datatable/datatable.component';
import { UserLocalStorageService } from '~/app/shared/services/user-local-storage.service';
import { TestingModule } from '~/app/testing.module';

describe('DatatableComponent', () => {
  let component: DatatableComponent;
  let fixture: ComponentFixture<DatatableComponent>;
  let userLocalStorageService: UserLocalStorageService;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [ComponentsModule, TestingModule, ToastrModule.forRoot()]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DatatableComponent);
    component = fixture.componentInstance;
    userLocalStorageService = TestBed.inject(UserLocalStorageService);
    // Stub the storage service so the tests neither touch the browser
    // local storage nor issue the backing RPC request.
    jest.spyOn(userLocalStorageService, 'set').mockImplementation(() => undefined);
    jest.spyOn(userLocalStorageService, 'get').mockReturnValue(null);
    fixture.detectChanges();
  });

  afterEach(() => {
    jest.restoreAllMocks();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  it('should include the current limit in the page size options', () => {
    const newFixture = TestBed.createComponent(DatatableComponent);
    newFixture.componentInstance.limit = 20;
    newFixture.detectChanges();
    expect(newFixture.componentInstance.pageSizeOptions).toEqual([10, 20, 25, 50, 100]);
  });

  it('should not duplicate a preset limit in the page size options', () => {
    const newFixture = TestBed.createComponent(DatatableComponent);
    newFixture.componentInstance.limit = 50;
    newFixture.detectChanges();
    expect(newFixture.componentInstance.pageSizeOptions).toEqual([10, 25, 50, 100]);
  });

  it('should update the limit and reset the offset on page size change', () => {
    component.offset = 3;
    component.onPageSizeChange(100);
    expect(component.limit).toBe(100);
    expect(component.offset).toBe(0);
  });

  it('should persist the page size as a global setting', () => {
    component.onPageSizeChange(50);
    expect(userLocalStorageService.set).toHaveBeenCalledWith('datatable_pagesize', '50');
  });

  it('should send limit -1 for remote paging when All is selected', () => {
    component.remotePaging = true;
    let params: any;
    component.loadDataEvent.subscribe((value) => (params = value));
    component.onPageSizeChange(0);
    expect(component.limit).toBe(0);
    expect(params.limit).toBe(-1);
  });

  it('should restore the persisted page size on init', () => {
    jest.spyOn(userLocalStorageService, 'get').mockReturnValue('50');
    const newFixture = TestBed.createComponent(DatatableComponent);
    newFixture.detectChanges();
    expect(newFixture.componentInstance.limit).toBe(50);
  });

  it('should not apply the stored page size when paging is disabled', () => {
    jest.spyOn(userLocalStorageService, 'get').mockReturnValue('50');
    const newFixture = TestBed.createComponent(DatatableComponent);
    newFixture.componentInstance.limit = 0;
    newFixture.detectChanges();
    expect(newFixture.componentInstance.limit).toBe(0);
  });
});
