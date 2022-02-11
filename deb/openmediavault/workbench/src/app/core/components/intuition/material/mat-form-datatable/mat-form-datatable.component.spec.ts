import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { ToastrModule } from 'ngx-toastr';

import { MatFormDatatableComponent } from '~/app/core/components/intuition/material/mat-form-datatable/mat-form-datatable.component';
import { ComponentsModule } from '~/app/shared/components/components.module';
import { TestingModule } from '~/app/testing.module';

describe('MatFormDatatableComponent', () => {
  let component: MatFormDatatableComponent;
  let fixture: ComponentFixture<MatFormDatatableComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [ComponentsModule, TestingModule, ToastrModule.forRoot()]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(MatFormDatatableComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
