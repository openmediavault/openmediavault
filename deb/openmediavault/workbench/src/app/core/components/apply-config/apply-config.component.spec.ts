import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { ToastrModule } from 'ngx-toastr';

import { ApplyConfigComponent } from '~/app/core/components/apply-config/apply-config.component';
import { ComponentsModule } from '~/app/core/components/components.module';
import { TestingModule } from '~/app/testing.module';

describe('ApplyConfigComponent', () => {
  let component: ApplyConfigComponent;
  let fixture: ComponentFixture<ApplyConfigComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [ComponentsModule, TestingModule, ToastrModule.forRoot()]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ApplyConfigComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
