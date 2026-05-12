import { HttpErrorResponsePipe } from '~/app/shared/pipes/http-error-response.pipe';

describe('ErrorResponsePipe', () => {
  it('create an instance', () => {
    const pipe = new HttpErrorResponsePipe();
    expect(pipe).toBeTruthy();
  });
});
