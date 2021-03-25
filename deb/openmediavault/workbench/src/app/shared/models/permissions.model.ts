import * as _ from 'lodash';

export type Role = 'admin' | 'user';

// eslint-disable-next-line no-shadow
export enum Roles {
  admin = 'admin',
  user = 'user'
}

export type Permissions = {
  role?: Array<Role>;
};

// eslint-disable-next-line @typescript-eslint/naming-convention
export const Permissions = {
  fromObject: (o: Record<any, any>): Permissions => {
    let role = _.get(o, 'role', []);
    if (_.isString(role)) {
      role = [role];
    }
    return { ...o, role };
  },

  fromJSON: (text: string): Permissions => Permissions.fromObject(JSON.parse(text)),

  toJSON: (permissions: Permissions): string => JSON.stringify(permissions),

  /**
   * Check if the given permissions match each other.
   *
   * @param lh The permissions to validate from.
   * @param rh The permissions to validate against.
   */
  validate: (lh: Permissions, rh: Permissions): boolean => {
    if (_.isEmpty(lh.role) || _.isEmpty(rh.role)) {
      return false;
    }
    const missedRoles = _.difference(rh.role, lh.role);
    return !missedRoles.length ? true : false;
  },

  /**
   * Check if the given permissions
   *
   * @param permissions The permissions to check.
   * @param role The role to check for.
   */
  hasRole: (permissions: Permissions, role: Role): boolean => permissions.role.includes(role)
};
