import { TestBed } from '@angular/core/testing';

import { DialogService } from '~/app/shared/services/dialog.service';
import { SharedModule } from '~/app/shared/shared.module';
import { TestingModule } from '~/app/testing.module';

describe('DialogService', () => {
  let service: DialogService;

  beforeEach(() => {
    TestBed.configureTestingModule({
      imports: [SharedModule, TestingModule]
    });
    service = TestBed.inject(DialogService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
