import { Permissions, Roles } from '~/app/shared/models/permissions.model';

describe('Permissions', () => {
  it('should convert to JSON', () => {
    const text: string = Permissions.toJSON({ role: [Roles.admin] });
    expect(text).toBe('{"role":["admin"]}');
  });

  it('should convert from JSON (1)', () => {
    const perm: Permissions = Permissions.fromJSON('{"role":["admin","user"]}');
    expect(perm).toEqual({ role: [Roles.admin, Roles.user] });
  });

  it('should convert from JSON (2)', () => {
    const perm: Permissions = Permissions.fromJSON('{"role":"admin"}');
    expect(perm).toEqual({ role: [Roles.admin] });
  });

  it('should convert from JSON (3)', () => {
    const perm: Permissions = Permissions.fromJSON('{}');
    expect(perm).toEqual({ role: [] });
  });

  it('should have permissions (1)', () => {
    const perm1: Permissions = Permissions.fromJSON('{"role":"admin"}');
    const perm2: Permissions = Permissions.fromObject({ role: Roles.admin });
    expect(Permissions.validate(perm1, perm2)).toBeTruthy();
  });

  it('should have permissions (2)', () => {
    const perm1: Permissions = { role: [Roles.admin, Roles.user] };
    const perm2: Permissions = Permissions.fromObject({ role: Roles.admin });
    expect(Permissions.validate(perm1, perm2)).toBeTruthy();
  });

  it('should not have permissions (1)', () => {
    const perm1: Permissions = Permissions.fromObject({ role: Roles.admin });
    const perm2: Permissions = Permissions.fromObject({ role: Roles.user });
    expect(Permissions.validate(perm1, perm2)).toBeFalsy();
  });

  it('should not have permissions (2)', () => {
    const perm1: Permissions = Permissions.fromObject({ role: Roles.admin });
    const perm2: Permissions = {};
    expect(Permissions.validate(perm1, perm2)).toBeFalsy();
  });

  it('should not have permissions (3)', () => {
    const perm1: Permissions = {};
    const perm2: Permissions = { role: [Roles.admin, Roles.user] };
    expect(Permissions.validate(perm1, perm2)).toBeFalsy();
  });
});
