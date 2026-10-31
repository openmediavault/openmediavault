import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { By } from '@angular/platform-browser';
import { DatatableComponent as NgxDatatableComponent } from '@siemens/ngx-datatable';
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

  it('should configure the tree column and emit tree actions', () => {
    const row = { id: 1 };
    component.columns = [{ name: 'Name', prop: 'name', isTreeColumn: true }];
    fixture.detectChanges();

    expect(component.columns[0].treeToggleTemplate).toBe(component.treeToggleTpl);

    const treeAction = jest.fn();
    component.treeActionEvent.subscribe(treeAction);
    const datatable: NgxDatatableComponent = fixture.debugElement
      .query(By.directive(NgxDatatableComponent))
      .injector.get(NgxDatatableComponent);
    datatable.treeAction.emit({ row });

    expect(treeAction).toHaveBeenCalledWith(row);
  });

  it('should show a rotating icon in the tree toggle while loading', () => {
    const toggle = (treeStatus: string) => {
      const view = component.treeToggleTpl.createEmbeddedView({
        cellContext: { treeStatus, onTreeAction: jest.fn() }
      });
      view.detectChanges();
      return view.rootNodes[0] as HTMLElement;
    };
    const loading = toggle('loading').querySelector('mat-icon');
    expect(loading.classList).toContain('omv-icon-rotate-360-infinite');
    expect(toggle('loading').hasAttribute('disabled')).toBe(true);
    expect(toggle('loading').getAttribute('aria-busy')).toBe('true');
    expect(toggle('loading').getAttribute('aria-label')).toContain('Loading ...');
    const expanded = toggle('expanded').querySelector('mat-icon');
    expect(expanded.classList).not.toContain('omv-icon-rotate-360-infinite');
    expect(toggle('expanded').hasAttribute('aria-busy')).toBe(false);
    expect(toggle('expanded').getAttribute('aria-label')).toContain('Expand/Collapse');
    expect(toggle('disabled').querySelector('mat-icon')).toBeNull();
  });

  it('should offer the configured page size', () => {
    const newFixture = TestBed.createComponent(DatatableComponent);
    newFixture.componentInstance.limit = 20;
    newFixture.detectChanges();
    expect(newFixture.componentInstance.pageSizeOptions).toEqual([10, 20, 25, 50, 100]);
  });

  it('should not duplicate a preset page size', () => {
    const newFixture = TestBed.createComponent(DatatableComponent);
    newFixture.componentInstance.limit = 50;
    newFixture.detectChanges();
    expect(newFixture.componentInstance.pageSizeOptions).toEqual([10, 25, 50, 100]);
  });

  it('should reset the page and save the chosen size', () => {
    component.offset = 3;
    component.onPageSizeChange(100);
    expect(component.limit).toBe(100);
    expect(component.offset).toBe(0);
    expect(userLocalStorageService.set).toHaveBeenCalledWith('datatable_pagesize', '100');
  });

  it('should restore a saved page size', () => {
    jest.spyOn(userLocalStorageService, 'get').mockReturnValue('50');
    const newFixture = TestBed.createComponent(DatatableComponent);
    newFixture.detectChanges();
    expect(newFixture.componentInstance.limit).toBe(50);
  });

  it('should restore the All option', () => {
    jest.spyOn(userLocalStorageService, 'get').mockReturnValue('0');
    const newFixture = TestBed.createComponent(DatatableComponent);
    newFixture.detectChanges();
    expect(newFixture.componentInstance.limit).toBe(0);
    expect(newFixture.componentInstance.pagingEnabled).toBe(true);
  });

  it('should ignore invalid saved page sizes', () => {
    jest.spyOn(userLocalStorageService, 'get').mockReturnValue('10.5');
    const newFixture = TestBed.createComponent(DatatableComponent);
    newFixture.detectChanges();
    expect(newFixture.componentInstance.limit).toBe(25);
  });

  it('should preserve disabled paging', () => {
    jest.spyOn(userLocalStorageService, 'get').mockReturnValue('50');
    const newFixture = TestBed.createComponent(DatatableComponent);
    newFixture.componentInstance.limit = 0;
    newFixture.detectChanges();
    expect(newFixture.componentInstance.limit).toBe(0);
    expect(newFixture.componentInstance.pagingEnabled).toBe(false);
  });

  it('should preserve tables without a footer', () => {
    jest.spyOn(userLocalStorageService, 'get').mockReturnValue('50');
    const newFixture = TestBed.createComponent(DatatableComponent);
    newFixture.componentInstance.hasFooter = false;
    newFixture.detectChanges();
    expect(newFixture.componentInstance.limit).toBe(25);
  });

  it('should request all rows in remote paging mode', () => {
    component.remotePaging = true;
    const callback = jest.fn();
    component.loadDataEvent.subscribe(callback);
    component.onPageSizeChange(0);
    expect(callback).toHaveBeenCalledWith({ offset: 0, limit: -1 });
  });

  it('should search job commands and tags', () => {
    const backup = { command: '/usr/local/bin/backup', comment: 'daily' };
    const sync = { command: '/usr/local/bin/rsync', comment: 'storage' };
    component.columns = [
      { name: 'Command', prop: 'command' },
      { name: 'Tags', prop: 'comment' }
    ];
    component.data = [backup, sync];
    component.searchFilter = 'storage';
    component.applySearchFilter();
    expect(component.rows).toEqual([sync]);
  });
});
