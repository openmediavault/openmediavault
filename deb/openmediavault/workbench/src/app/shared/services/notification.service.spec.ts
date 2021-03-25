import { TestBed } from '@angular/core/testing';
import { ToastrModule } from 'ngx-toastr';

import { NotificationService } from '~/app/shared/services/notification.service';

describe('NotificationService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [ToastrModule.forRoot()]
    })
  );

  it('should be created', () => {
    const service: NotificationService = TestBed.inject(NotificationService);
    expect(service).toBeTruthy();
  });
});
