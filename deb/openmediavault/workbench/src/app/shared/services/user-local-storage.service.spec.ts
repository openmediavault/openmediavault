import { TestBed } from '@angular/core/testing';

import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { UserLocalStorageService } from '~/app/shared/services/user-local-storage.service';

describe('UserLocalStorageService', () => {
  const username = 'admin';
  let service: UserLocalStorageService;
  let authSessionService: AuthSessionService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(UserLocalStorageService);
    authSessionService = TestBed.inject(AuthSessionService);
    jest.spyOn(authSessionService, 'getUsername').mockReturnValue(username);
    localStorage.clear();
  });

  afterEach(() => {
    localStorage.clear();
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });

  it('should remove', () => {
    const itemName = 'test_item';
    const itemValue = 'test_value';
    localStorage.setItem(`${username}@${itemName}`, itemValue);
    expect(localStorage.length).toBe(1);
    service.remove(itemName);
    expect(localStorage.length).toBe(0);
  });

  it('should clear', () => {
    const itemName = 'test_item';
    const itemValue = 'test_value';
    localStorage.setItem(`${username}@${itemName}_1`, itemValue);
    localStorage.setItem(`${username}@${itemName}_2`, itemValue);
    expect(localStorage.length).toBe(2);
    service.clear();
    expect(localStorage.length).toBe(0);
  });

  it('should set', () => {
    const itemName = 'test_item';
    const itemValue = 'test_value';
    expect(localStorage.length).toBe(0);
    service.set(itemName, itemValue);
    expect(localStorage.length).toBe(1);
    expect(localStorage.getItem(`${username}@${itemName}`)).toBe(itemValue);
  });

  it('should get', () => {
    const itemName = 'test_item';
    const itemValue = 'test_value';
    localStorage.setItem(`${username}@${itemName}`, itemValue);
    expect(localStorage.length).toBe(1);
    const result = service.get(itemName);
    expect(localStorage.length).toBe(1);
    expect(itemValue).toBe(result);
  });
});
