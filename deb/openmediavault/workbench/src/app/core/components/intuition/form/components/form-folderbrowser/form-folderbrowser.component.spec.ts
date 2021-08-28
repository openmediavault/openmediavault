import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { TranslateModule } from '@ngx-translate/core';

// eslint-disable-next-line max-len
import { FormFolderbrowserComponent } from '~/app/core/components/intuition/form/components/form-folderbrowser/form-folderbrowser.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';

describe('FormFolderbrowserComponent', () => {
  let component: FormFolderbrowserComponent;
  let fixture: ComponentFixture<FormFolderbrowserComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [
          HttpClientTestingModule,
          IntuitionModule,
          NoopAnimationsModule,
          TranslateModule.forRoot()
        ]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(FormFolderbrowserComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'folderBrowser',
      name: 'foo',
      dirRefIdField: 'bar'
    };
    const formBuilder = TestBed.inject(FormBuilder);
    component.formGroup = formBuilder.group({ foo: [''], bar: [''] });
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
