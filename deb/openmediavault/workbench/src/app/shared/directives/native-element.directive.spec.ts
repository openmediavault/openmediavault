import { NativeElementDirective } from '~/app/shared/directives/native-element.directive';

describe('NativeElementDirective', () => {
  it('should create an instance', () => {
    const directive = new NativeElementDirective(null, null);
    expect(directive).toBeTruthy();
  });
});
