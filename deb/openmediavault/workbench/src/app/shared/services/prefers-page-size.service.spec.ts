import { TestBed } from '@angular/core/testing';

import { PrefersPageSizeService } from '~/app/shared/services/prefers-page-size.service';
import { TestingModule } from '~/app/testing.module';

describe('PrefersPageSizeService', () => {
  let service: PrefersPageSizeService;

  beforeEach(() => {
    TestBed.configureTestingModule({
      imports: [TestingModule]
    });
    service = TestBed.inject(PrefersPageSizeService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
