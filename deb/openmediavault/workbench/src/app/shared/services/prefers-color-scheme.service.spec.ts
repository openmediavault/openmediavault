import { TestBed } from '@angular/core/testing';

import { PrefersColorSchemeService } from '~/app/shared/services/prefers-color-scheme.service';
import { TestingModule } from '~/app/testing.module';

describe('PrefersColorSchemeService', () => {
  let service: PrefersColorSchemeService;

  beforeEach(() => {
    TestBed.configureTestingModule({
      imports: [TestingModule]
    });
    service = TestBed.inject(PrefersColorSchemeService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
