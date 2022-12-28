import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { FormBuilder } from '@angular/forms';
import { ToastrModule } from 'ngx-toastr';

import { FormCodeEditorComponent } from '~/app/core/components/intuition/form/components/form-code-editor/form-code-editor.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { TestingModule } from '~/app/testing.module';

describe('FormCodeEditorComponent', () => {
  let component: FormCodeEditorComponent;
  let fixture: ComponentFixture<FormCodeEditorComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      declarations: [FormCodeEditorComponent],
      imports: [IntuitionModule, TestingModule, ToastrModule.forRoot()]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FormCodeEditorComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'codeEditor',
      name: 'foo',
      language: 'yaml',
      lineNumbers: false
    };
    const formBuilder = TestBed.inject(FormBuilder);
    component.formGroup = formBuilder.group({ foo: ['xyz'] });
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
