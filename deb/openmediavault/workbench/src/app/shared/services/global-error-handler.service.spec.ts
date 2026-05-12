import { TestBed } from '@angular/core/testing';

import { GlobalErrorHandlerService } from '~/app/shared/services/global-error-handler.service';
import { TestingModule } from '~/app/testing.module';

describe('GlobalErrorHandlerService', () => {
  let service: GlobalErrorHandlerService;

  beforeEach(() => {
    TestBed.configureTestingModule({
      imports: [TestingModule]
    });
    service = TestBed.inject(GlobalErrorHandlerService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
