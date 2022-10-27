import { TestBed } from '@angular/core/testing';

import { TitleService } from '~/app/shared/services/title.service';
import { TestingModule } from '~/app/testing.module';

describe('TitleService', () => {
  let service: TitleService;

  beforeEach(() => {
    TestBed.configureTestingModule({
      imports: [TestingModule]
    });
    service = TestBed.inject(TitleService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
