import { TestBed } from '@angular/core/testing';

import { IsDirtyGuardService } from '~/app/shared/services/is-dirty-guard.service';
import { SharedModule } from '~/app/shared/shared.module';
import { TestingModule } from '~/app/testing.module';

describe('IsDirtyGuardService', () => {
  let service: IsDirtyGuardService;

  beforeEach(() => {
    TestBed.configureTestingModule({
      imports: [SharedModule, TestingModule]
    });
    service = TestBed.inject(IsDirtyGuardService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
