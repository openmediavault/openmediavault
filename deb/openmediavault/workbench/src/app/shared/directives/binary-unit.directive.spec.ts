import { BinaryUnitDirective } from '~/app/shared/directives/binary-unit.directive';

export class MockElementRef {
  nativeElement = {};
}

describe('BinaryUnitDirective', () => {
  it('should create an instance', () => {
    const directive = new BinaryUnitDirective(new MockElementRef(), null);
    expect(directive).toBeTruthy();
  });
});
