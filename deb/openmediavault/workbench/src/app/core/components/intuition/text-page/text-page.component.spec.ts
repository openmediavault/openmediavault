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

  describe('content sanitizing', () => {
    const render = (contentFormat: string, value: string): HTMLElement => {
      component.config.contentFormat = contentFormat as any;
      component.config.request = { service: 'Foo', get: { method: 'bar' } };
      (component as any).onLoadData(value);
      return (component as any)._contentContainer.nativeElement;
    };

    it('should remove scripts and event handlers from markdown content', () => {
      const el = render(
        'markdown',
        '# Title\n\n<img src="x" onerror="alert(1)"><script>alert(2)</script>'
      );
      expect(el.querySelector('h1')?.textContent).toBe('Title');
      expect(el.innerHTML).not.toContain('onerror');
      expect(el.querySelector('script')).toBeNull();
    });

    it('should remove scripts and event handlers from html content', () => {
      const el = render(
        'html',
        '<p>Text</p><a href="javascript:alert(1)">x</a><b onclick="alert(2)">y</b>'
      );
      expect(el.querySelector('p')?.textContent).toBe('Text');
      expect(el.innerHTML).not.toContain('onclick');
      // Angular neutralizes unsafe URLs by prefixing them with `unsafe:`.
      expect(el.querySelector('a')?.getAttribute('href')).toMatch(/^unsafe:/);
    });

    it('should render plain text content as text', () => {
      const el = render('plain', '<b>bold</b>');
      expect(el.textContent).toBe('<b>bold</b>');
      expect(el.querySelector('b')).toBeNull();
    });
  });
});
