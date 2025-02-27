import { TestBed } from '@angular/core/testing';

import { RouteContextService } from '~/app/shared/services/route-context.service';
import { TestingModule } from '~/app/testing.module';

describe('RouteContextService', () => {
  let service: RouteContextService;

  beforeEach(() => {
    TestBed.configureTestingModule({
      imports: [TestingModule]
    });
    service = TestBed.inject(RouteContextService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
