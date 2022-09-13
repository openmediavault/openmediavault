import { TestBed } from '@angular/core/testing';

import { PrefersColorSchemeService } from '~/app/shared/services/prefers-color-scheme.service';

describe('PrefersColorSchemeService', () => {
  let service: PrefersColorSchemeService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(PrefersColorSchemeService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
