import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { ComponentsModule } from '~/app/core/components/components.module';
import { NavigationBarComponent } from '~/app/core/components/navigation-bar/navigation-bar.component';
import { TestingModule } from '~/app/testing.module';

describe('NavigationBarComponent', () => {
  let component: NavigationBarComponent;
  let fixture: ComponentFixture<NavigationBarComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [ComponentsModule, TestingModule]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NavigationBarComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
