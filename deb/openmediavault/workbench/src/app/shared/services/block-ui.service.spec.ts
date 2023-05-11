import { TestBed } from '@angular/core/testing';

import { BlockUiService } from '~/app/shared/services/block-ui.service';

describe('BlockUiService', () => {
  let service: BlockUiService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(BlockUiService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
