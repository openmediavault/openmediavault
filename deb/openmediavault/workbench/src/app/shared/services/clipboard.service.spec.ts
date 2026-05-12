import { TestBed } from '@angular/core/testing';
import { ToastrModule } from 'ngx-toastr';

import { ClipboardService } from '~/app/shared/services/clipboard.service';

describe('ClipboardService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [ToastrModule.forRoot()]
    })
  );

  it('should be created', () => {
    const service: ClipboardService = TestBed.inject(ClipboardService);
    expect(service).toBeTruthy();
  });
});
