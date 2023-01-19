import { TestBed } from '@angular/core/testing';

import { UserLocalStorageService } from '~/app/shared/services/user-local-storage.service';
import { TestingModule } from '~/app/testing.module';

describe('UserLocalStorageService', () => {
  let service: UserLocalStorageService;

  beforeEach(() => {
    TestBed.configureTestingModule({
      imports: [TestingModule]
    });
    service = TestBed.inject(UserLocalStorageService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
