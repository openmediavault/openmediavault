import { HttpClientTestingModule } from '@angular/common/http/testing';
import { TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';

import { SystemInformationService } from '~/app/shared/services/system-information.service';

describe('SystemInformationService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [HttpClientTestingModule, RouterTestingModule]
    })
  );

  it('should be created', () => {
    const service: SystemInformationService = TestBed.inject(SystemInformationService);
    expect(service).toBeTruthy();
  });
});
