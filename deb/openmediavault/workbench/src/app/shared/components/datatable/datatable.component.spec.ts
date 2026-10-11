import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { By } from '@angular/platform-browser';
import { DatatableComponent as NgxDatatableComponent } from '@siemens/ngx-datatable';
import { ToastrModule } from 'ngx-toastr';

import { ComponentsModule } from '~/app/shared/components/components.module';
import { DatatableComponent } from '~/app/shared/components/datatable/datatable.component';
import { TestingModule } from '~/app/testing.module';

describe('DatatableComponent', () => {
  let component: DatatableComponent;
  let fixture: ComponentFixture<DatatableComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [ComponentsModule, TestingModule, ToastrModule.forRoot()]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DatatableComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
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
});
