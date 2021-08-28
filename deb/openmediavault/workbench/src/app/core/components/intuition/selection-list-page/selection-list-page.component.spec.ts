import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { ToastrModule } from 'ngx-toastr';

import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { SelectionListPageComponent } from '~/app/core/components/intuition/selection-list-page/selection-list-page.component';

describe('SelectionListPageComponent', () => {
  let component: SelectionListPageComponent;
  let fixture: ComponentFixture<SelectionListPageComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [
          HttpClientTestingModule,
          IntuitionModule,
          RouterTestingModule,
          ToastrModule.forRoot()
        ]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(SelectionListPageComponent);
    component = fixture.componentInstance;
    component.config = {
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
