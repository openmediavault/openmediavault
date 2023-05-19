import { TestBed } from '@angular/core/testing';
import { ToastrModule } from 'ngx-toastr';

import { HttpErrorInterceptorService } from '~/app/shared/services/http-error-interceptor.service';
import { TestingModule } from '~/app/testing.module';

describe('HttpErrorInterceptorService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [TestingModule, ToastrModule.forRoot()]
    })
  );

  it('should be created', () => {
    const service: HttpErrorInterceptorService = TestBed.inject(HttpErrorInterceptorService);
    expect(service).toBeTruthy();
  });
});
