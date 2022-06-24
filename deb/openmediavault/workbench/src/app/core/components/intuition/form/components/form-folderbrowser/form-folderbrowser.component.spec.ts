import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';

// eslint-disable-next-line max-len
import { FormFolderbrowserComponent } from '~/app/core/components/intuition/form/components/form-folderbrowser/form-folderbrowser.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { TestingModule } from '~/app/testing.module';

describe('FormFolderbrowserComponent', () => {
  let component: FormFolderbrowserComponent;
  let fixture: ComponentFixture<FormFolderbrowserComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [IntuitionModule, TestingModule]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FormFolderbrowserComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'folderBrowser',
      name: 'foo',
      dirRefIdField: 'bar',
      dirVisible: false
    };
    const formBuilder = TestBed.inject(FormBuilder);
    component.formGroup = formBuilder.group({ foo: [''], bar: [''] });
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
