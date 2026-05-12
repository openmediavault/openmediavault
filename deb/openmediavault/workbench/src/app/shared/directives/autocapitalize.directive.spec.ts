import { AutocapitalizeDirective } from '~/app/shared/directives/autocapitalize.directive';

describe('AutocapitalizeDirective', () => {
  it('should create an instance', () => {
    const directive = new AutocapitalizeDirective(null);
    expect(directive).toBeTruthy();
  });
});
