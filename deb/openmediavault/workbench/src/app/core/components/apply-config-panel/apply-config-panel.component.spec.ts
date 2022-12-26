import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { ToastrModule } from 'ngx-toastr';

import { ApplyConfigPanelComponent } from '~/app/core/components/apply-config-panel/apply-config-panel.component';
import { ComponentsModule } from '~/app/core/components/components.module';
import { TestingModule } from '~/app/testing.module';

describe('ApplyConfigPanelComponent', () => {
  let component: ApplyConfigPanelComponent;
  let fixture: ComponentFixture<ApplyConfigPanelComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [ComponentsModule, TestingModule, ToastrModule.forRoot()]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ApplyConfigPanelComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
