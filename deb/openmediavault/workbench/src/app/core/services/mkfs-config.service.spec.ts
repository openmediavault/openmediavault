import { TestBed } from '@angular/core/testing';

import { MkfsConfigService } from '~/app/core/services/mkfs-config.service';
import { TestingModule } from '~/app/testing.module';

describe('MkfsConfigService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [TestingModule]
    })
  );

  it('should be created', () => {
    const service: MkfsConfigService = TestBed.inject(MkfsConfigService);
    expect(service).toBeTruthy();
  });
});
