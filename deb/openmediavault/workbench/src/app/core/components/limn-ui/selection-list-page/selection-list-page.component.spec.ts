import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { ToastrModule } from 'ngx-toastr';

import { LimnUiModule } from '~/app/core/components/limn-ui/limn-ui.module';
import { SelectionListPageComponent } from '~/app/core/components/limn-ui/selection-list-page/selection-list-page.component';

describe('SelectionListPageComponent', () => {
  let component: SelectionListPageComponent;
  let fixture: ComponentFixture<SelectionListPageComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [
          HttpClientTestingModule,
          LimnUiModule,
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
