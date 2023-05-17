import { TestBed } from '@angular/core/testing';

import { RpcService } from '~/app/shared/services/rpc.service';
import { TestingModule } from '~/app/testing.module';

describe('RpcService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [TestingModule],
      teardown: { destroyAfterEach: false }
    })
  );

  it('should be created', () => {
    const service: RpcService = TestBed.inject(RpcService);
    expect(service).toBeTruthy();
  });
});
