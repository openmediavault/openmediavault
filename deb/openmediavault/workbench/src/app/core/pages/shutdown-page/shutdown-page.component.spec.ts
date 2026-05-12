import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { PagesModule } from '~/app/core/pages/pages.module';
import { ShutdownPageComponent } from '~/app/core/pages/shutdown-page/shutdown-page.component';
import { TestingModule } from '~/app/testing.module';

describe('ShutdownPageComponent', () => {
  let component: ShutdownPageComponent;
  let fixture: ComponentFixture<ShutdownPageComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [PagesModule, TestingModule]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ShutdownPageComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
