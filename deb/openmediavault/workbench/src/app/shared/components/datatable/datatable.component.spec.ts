import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { TranslateModule } from '@ngx-translate/core';
import { ToastrModule } from 'ngx-toastr';

import { ComponentsModule } from '~/app/shared/components/components.module';
import { DatatableComponent } from '~/app/shared/components/datatable/datatable.component';

describe('DatatableComponent', () => {
  let component: DatatableComponent;
  let fixture: ComponentFixture<DatatableComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [
          ComponentsModule,
          HttpClientTestingModule,
          ToastrModule.forRoot(),
          TranslateModule.forRoot()
        ]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(DatatableComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
