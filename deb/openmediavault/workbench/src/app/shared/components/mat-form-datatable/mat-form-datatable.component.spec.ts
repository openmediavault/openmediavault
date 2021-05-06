import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { ToastrModule } from 'ngx-toastr';

import { ComponentsModule } from '~/app/shared/components/components.module';
import { MatFormDatatableComponent } from '~/app/shared/components/mat-form-datatable/mat-form-datatable.component';

describe('MatFormDatatableComponent', () => {
  let component: MatFormDatatableComponent;
  let fixture: ComponentFixture<MatFormDatatableComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [ComponentsModule, HttpClientTestingModule, ToastrModule.forRoot()]
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
