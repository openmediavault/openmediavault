import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { ComponentsModule } from '~/app/shared/components/components.module';
import { DatatableActionsComponent } from '~/app/shared/components/datatable-actions/datatable-actions.component';
import { TestingModule } from '~/app/testing.module';

describe('DatatableActionsComponent', () => {
  let component: DatatableActionsComponent;
  let fixture: ComponentFixture<DatatableActionsComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [ComponentsModule, TestingModule]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(DatatableActionsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
