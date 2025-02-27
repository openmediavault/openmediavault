import { TestBed } from '@angular/core/testing';

import { PwaUpdateService } from '~/app/core/services/pwa-update.service';
import { TestingModule } from '~/app/testing.module';

describe('PwaUpdateService', () => {
  let service: PwaUpdateService;

  beforeEach(() => {
    TestBed.configureTestingModule({
      imports: [TestingModule]
    });
    service = TestBed.inject(PwaUpdateService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});