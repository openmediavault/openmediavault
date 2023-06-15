import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';
import { ToastrModule } from 'ngx-toastr';

import { FormDatatableComponent } from '~/app/core/components/intuition/form/components/form-datatable/form-datatable.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { TestingModule } from '~/app/testing.module';

describe('FormDatatableComponent', () => {
  let component: FormDatatableComponent;
  let fixture: ComponentFixture<FormDatatableComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [IntuitionModule, TestingModule, ToastrModule.forRoot()]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FormDatatableComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'datatable',
      name: 'foo',
      columnMode: 'flex',
      hasHeader: true,
      hasFooter: true,
      selectionType: 'multi',
      limit: 25,
      columns: [],
      actions: [],
      sorters: [],
      sortType: 'single',
      valueType: 'object'
    };
    const formBuilder = TestBed.inject(FormBuilder);
    component.formGroup = formBuilder.group({ foo: [[]] });
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
