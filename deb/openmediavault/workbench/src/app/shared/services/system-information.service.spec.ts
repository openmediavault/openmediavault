import { TestBed } from '@angular/core/testing';

import { SystemInformationService } from '~/app/shared/services/system-information.service';
import { TestingModule } from '~/app/testing.module';

describe('SystemInformationService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [TestingModule]
    })
  );

  it('should be created', () => {
    const service: SystemInformationService = TestBed.inject(SystemInformationService);
    expect(service).toBeTruthy();
  });
});
