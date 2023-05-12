import { TestBed, waitForAsync } from '@angular/core/testing';

import { AppComponent } from '~/app/app.component';
import { SharedModule } from '~/app/shared/shared.module';
import { TestingModule } from '~/app/testing.module';

describe('AppComponent', () => {
  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      declarations: [AppComponent],
      imports: [SharedModule, TestingModule]
    }).compileComponents();
  }));

  it('should create the app', () => {
    const fixture = TestBed.createComponent(AppComponent);
    const app = fixture.debugElement.componentInstance;
    expect(app).toBeTruthy();
  });
});
