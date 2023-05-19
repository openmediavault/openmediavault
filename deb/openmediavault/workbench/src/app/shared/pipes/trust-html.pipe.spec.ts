import { TestBed } from '@angular/core/testing';
import { DomSanitizer } from '@angular/platform-browser';

import { TrustHtmlPipe } from '~/app/shared/pipes/trust-html.pipe';

describe('TrustHtmlPipe', () => {
  let pipe: TrustHtmlPipe;
  let domSanitizer: DomSanitizer;

  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [DomSanitizer]
    });
    domSanitizer = TestBed.inject(DomSanitizer);
    pipe = new TrustHtmlPipe(domSanitizer);
  });

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });
});
