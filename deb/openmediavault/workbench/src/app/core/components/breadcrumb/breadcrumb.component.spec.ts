import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { BreadcrumbComponent } from '~/app/core/components/breadcrumb/breadcrumb.component';
import { ComponentsModule } from '~/app/core/components/components.module';
import { TestingModule } from '~/app/testing.module';

describe('BreadcrumbComponent', () => {
  let component: BreadcrumbComponent;
  let fixture: ComponentFixture<BreadcrumbComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [ComponentsModule, TestingModule]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(BreadcrumbComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
