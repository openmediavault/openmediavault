import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { ToastrModule } from 'ngx-toastr';

import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { TextPageComponent } from '~/app/core/components/intuition/text-page/text-page.component';
import { TestingModule } from '~/app/testing.module';

describe('TextPageComponent', () => {
  let component: TextPageComponent;
  let fixture: ComponentFixture<TextPageComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [IntuitionModule, TestingModule, ToastrModule.forRoot()]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TextPageComponent);
    component = fixture.componentInstance;
    component.config = {};
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  it('should render title tokens with the page context', () => {
    // The titles are not formatted in the configuration, the template does it.
    component.config.title = 'Title {{ name }}';
    component.config.subTitle = 'Subtitle {{ name }}';
    component.pageContextService.set({ name: 'Test' });
    fixture.detectChanges();

    expect(fixture.nativeElement.querySelector('mat-card-title').textContent).toContain(
      'Title Test'
    );
    expect(fixture.nativeElement.querySelector('mat-card-subtitle').textContent).toContain(
      'Subtitle Test'
    );
  });
});
