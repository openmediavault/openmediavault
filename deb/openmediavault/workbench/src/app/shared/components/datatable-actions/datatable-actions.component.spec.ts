import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { ComponentsModule } from '~/app/shared/components/components.module';
import { DatatableActionsComponent } from '~/app/shared/components/datatable-actions/datatable-actions.component';
import { DatatableSelection } from '~/app/shared/models/datatable-selection.model';
import { TestingModule } from '~/app/testing.module';

describe('DatatableActionsComponent', () => {
  let component: DatatableActionsComponent;
  let fixture: ComponentFixture<DatatableActionsComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [ComponentsModule, TestingModule]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DatatableActionsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  it('should test isDisabled() [1]', () => {
    expect(
      component.isDisabled({
        enabledConstraints: undefined
      })
    ).toBeFalsy();
  });

  it('should test isDisabled() [2]', () => {
    // @ts-ignore
    component.table = {};
    expect(
      component.isDisabled({
        enabledConstraints: {
          hasData: true
        }
      })
    ).toBeTruthy();
  });

  it('should test isDisabled() [3]', () => {
    // @ts-ignore
    component.table = {};
    expect(
      component.isDisabled({
        enabledConstraints: {
          hasData: false
        }
      })
    ).toBeFalsy();
  });

  it('should test isDisabled() [4]', () => {
    // @ts-ignore
    component.table = { data: [1, 2, 3] };
    expect(
      component.isDisabled({
        enabledConstraints: {
          hasData: true
        }
      })
    ).toBeFalsy();
  });

  it('should test isDisabled() [5]', () => {
    // @ts-ignore
    component.table = { data: [1, 2] };
    component.selection = new DatatableSelection();
    expect(
      component.isDisabled({
        enabledConstraints: {
          minSelected: 1
        }
      })
    ).toBeTruthy();
  });

  it('should test isDisabled() [6]', () => {
    // @ts-ignore
    component.table = { data: [1, 2] };
    component.selection = new DatatableSelection();
    component.selection.set([1, 2]);
    expect(
      component.isDisabled({
        enabledConstraints: {
          minSelected: 1
        }
      })
    ).toBeFalsy();
  });

  it('should test isDisabled() [7]', () => {
    // @ts-ignore
    component.table = { data: [1, 2] };
    component.selection = new DatatableSelection();
    component.selection.set([1]);
    expect(
      component.isDisabled({
        enabledConstraints: {
          maxSelected: 1
        }
      })
    ).toBeFalsy();
  });

  it('should test isDisabled() [8]', () => {
    // @ts-ignore
    component.table = { data: [1, 2] };
    component.selection = new DatatableSelection();
    component.selection.set([1, 2]);
    expect(
      component.isDisabled({
        enabledConstraints: {
          maxSelected: 1
        }
      })
    ).toBeTruthy();
  });
});
