import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { PagesModule } from '~/app/core/pages/pages.module';
import { StandbyPageComponent } from '~/app/core/pages/standby-page/standby-page.component';
import { TestingModule } from '~/app/testing.module';

describe('StandbyPageComponent', () => {
  let component: StandbyPageComponent;
  let fixture: ComponentFixture<StandbyPageComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [PagesModule, TestingModule]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(StandbyPageComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
