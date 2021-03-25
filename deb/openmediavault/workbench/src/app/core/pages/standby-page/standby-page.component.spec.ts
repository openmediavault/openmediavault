import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { PagesModule } from '~/app/core/pages/pages.module';
import { StandbyPageComponent } from '~/app/core/pages/standby-page/standby-page.component';

describe('StandbyPageComponent', () => {
  let component: StandbyPageComponent;
  let fixture: ComponentFixture<StandbyPageComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [PagesModule, HttpClientTestingModule]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(StandbyPageComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
