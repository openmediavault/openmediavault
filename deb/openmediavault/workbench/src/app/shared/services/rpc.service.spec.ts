import { HttpClientTestingModule } from '@angular/common/http/testing';
import { TestBed } from '@angular/core/testing';

import { RpcService } from '~/app/shared/services/rpc.service';

describe('RpcService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [HttpClientTestingModule]
    })
  );

  it('should be created', () => {
    const service: RpcService = TestBed.inject(RpcService);
    expect(service).toBeTruthy();
  });
});
