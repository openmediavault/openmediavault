import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { FormComponent } from '~/app/core/components/intuition/form/form.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { PageContextService } from '~/app/core/services/page-context.service';
import { TestingModule } from '~/app/testing.module';

describe('FormComponent', () => {
  let component: FormComponent;
  let fixture: ComponentFixture<FormComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [IntuitionModule, TestingModule],
      providers: [PageContextService]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FormComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
