import { TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { ToastrModule } from 'ngx-toastr';

import { HttpErrorInterceptorService } from '~/app/shared/services/http-error-interceptor.service';

describe('HttpErrorInterceptorService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [RouterTestingModule, ToastrModule.forRoot()]
    })
  );

  it('should be created', () => {
    const service: HttpErrorInterceptorService = TestBed.inject(HttpErrorInterceptorService);
    expect(service).toBeTruthy();
  });
});
