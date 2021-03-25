import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { TranslateModule } from '@ngx-translate/core';
import { ToastrModule } from 'ngx-toastr';

import { DatatablePageComponent } from '~/app/core/components/limn-ui/datatable-page/datatable-page.component';
import { LimnUiModule } from '~/app/core/components/limn-ui/limn-ui.module';

describe('DatatablePageComponent', () => {
  let component: DatatablePageComponent;
  let fixture: ComponentFixture<DatatablePageComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [
          HttpClientTestingModule,
          LimnUiModule,
          RouterTestingModule,
          ToastrModule.forRoot(),
          TranslateModule.forRoot()
        ]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(DatatablePageComponent);
    component = fixture.componentInstance;
    component.config = {
      columns: [],
      store: {
        data: []
      }
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
