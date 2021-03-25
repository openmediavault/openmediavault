import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { FormParagraphComponent } from '~/app/core/components/limn-ui/form/components/form-paragraph/form-paragraph.component';
import { LimnUiModule } from '~/app/core/components/limn-ui/limn-ui.module';

describe('FormParagraphComponent', () => {
  let component: FormParagraphComponent;
  let fixture: ComponentFixture<FormParagraphComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [LimnUiModule]
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
