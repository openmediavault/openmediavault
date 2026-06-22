import { ComponentFixture, TestBed } from '@angular/core/testing';

import { ComponentsModule } from '~/app/core/components/components.module';
import { AuthenticationLayoutComponent } from '~/app/core/components/layouts/authentication-layout/authentication-layout.component';
import { TestingModule } from '~/app/testing.module';

describe('AuthenticationLayoutComponent', () => {
  let component: AuthenticationLayoutComponent;
  let fixture: ComponentFixture<AuthenticationLayoutComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [ComponentsModule, TestingModule]
    }).compileComponents();

    fixture = TestBed.createComponent(AuthenticationLayoutComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
