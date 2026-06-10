import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { ComponentsModule } from '~/app/core/components/components.module';
import { BackgroundImageLayoutComponent } from '~/app/core/components/layouts/background-image-layout/background-image-layout.component';
import { TestingModule } from '~/app/testing.module';

describe('BackgroundImageLayoutComponent', () => {
  let component: BackgroundImageLayoutComponent;
  let fixture: ComponentFixture<BackgroundImageLayoutComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [ComponentsModule, TestingModule]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(BackgroundImageLayoutComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
