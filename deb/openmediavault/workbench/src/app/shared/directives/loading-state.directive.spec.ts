import { LoadingStateDirective } from '~/app/shared/directives/loading-state.directive';

describe('LoadingStateDirective', () => {
  it('should create an instance', () => {
    const directive = new LoadingStateDirective(null, null);
    expect(directive).toBeTruthy();
  });
});
