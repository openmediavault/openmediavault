import { TestBed } from '@angular/core/testing';

import { UserLocalStorageService } from '~/app/shared/services/user-local-storage.service';

describe('UserLocalStorageService', () => {
  let service: UserLocalStorageService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(UserLocalStorageService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
