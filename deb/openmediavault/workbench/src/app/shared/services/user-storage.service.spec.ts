import { TestBed } from '@angular/core/testing';

import { UserStorageService } from '~/app/shared/services/user-storage.service';

describe('UserStorageService', () => {
  let service: UserStorageService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(UserStorageService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
