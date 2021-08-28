import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { FormParagraphComponent } from '~/app/core/components/intuition/form/components/form-paragraph/form-paragraph.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';

describe('FormParagraphComponent', () => {
  let component: FormParagraphComponent;
  let fixture: ComponentFixture<FormParagraphComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [IntuitionModule]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(FormParagraphComponent);
    component = fixture.componentInstance;
    component.config = {
      type: 'paragraph'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
