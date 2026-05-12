import { TestBed } from '@angular/core/testing';

import { LocaleService } from '~/app/shared/services/locale.service';

describe('LocaleService', () => {
  beforeEach(() => TestBed.configureTestingModule({}));

  it('should be created', () => {
    const service: LocaleService = TestBed.inject(LocaleService);
    expect(service).toBeTruthy();
  });
});
