import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { ToastrModule } from 'ngx-toastr';

import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { MatFormDatatableComponent } from '~/app/core/components/intuition/material/mat-form-datatable/mat-form-datatable.component';
import { TestingModule } from '~/app/testing.module';

describe('MatFormDatatableComponent', () => {
  let component: MatFormDatatableComponent;
  let fixture: ComponentFixture<MatFormDatatableComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      declarations: [MatFormDatatableComponent],
      imports: [IntuitionModule, TestingModule, ToastrModule.forRoot()]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(MatFormDatatableComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
