/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
import {
  AbstractControl,
  FormArray,
  FormGroup,
  ValidationErrors,
  ValidatorFn,
  Validators
} from '@angular/forms';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { debounceTime } from 'rxjs/operators';
import validator from 'validator';

import { format, formatDeep, toBytes } from '~/app/functions.helper';
import { Constraint } from '~/app/shared/models/constraint.type';
import { ConstraintService } from '~/app/shared/services/constraint.service';

const regExp = {
  /* eslint-disable max-len */
  // Taken from Debian adduser command.
  userName: /^[_.A-Za-z0-9][-\@_.A-Za-z0-9]*\$?$/,
  groupName: /^[a-zA-Z0-9\-\.]+$/,
  // We are using the SMB/CIFS file/directory naming convention for this:
  // All characters are legal in the basename and extension except the
  // space character (0x20) and:
  // "./\[]:+|<>=;,*?
  // A share name or server or workstation name SHOULD not begin with a
  // period (“.”) nor should it include two adjacent periods (“..”).
  // References:
  // http://tools.ietf.org/html/draft-leach-cifs-v1-spec-01
  // http://msdn.microsoft.com/en-us/library/aa365247%28VS.85%29.aspx
  shareName: /^[^.]([^"/\\\[\]:+|<>=;,*?. ]+){0,1}([.][^"/\\\[\]:+|<>=;,*?. ]+){0,}$/,
  ipv4NetCidr:
    /^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/(3[0-2]|[0-2]?[0-9])$/,
  ipv6NetCidr: /^(?:[a-f0-9]{1,4}:){7}[a-f0-9]{1,4}\/(12[0-8]|1[0-1][0-9]|[1-9][0-9]|[0-9])$/i,
  // See https://datatracker.ietf.org/doc/html/rfc1123#section-2
  hostName: /^[a-zA-Z0-9]([-a-zA-Z0-9]{0,61}[a-zA-Z0-9]){0,1}$/,
  // See http://shauninman.com/archive/2006/05/08/validating_domain_names
  domainName:
    /^[a-zA-Z0-9]([-a-zA-Z0-9]{0,61}[a-zA-Z0-9])?([.][a-zA-Z0-9]([-a-zA-Z0-9]{0,61}[a-zA-Z0-9])?)*$/,
  // https://github.com/samba-team/samba/blob/samba-4.13.13/python/samba/__init__.py#L317
  netbiosName: /^[\w !#$%&'()\-.@^_{}~]{1,15}$/,
  port: /^([1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$/,
  numeric: /^[-]?\d+$/,
  decimal: /^[-]?\d+(.\d+)?$/,
  time: /^\d{2}:\d{2}:\d{2}$/,
  // See https://tools.ietf.org/html/rfc4716#section-3.4
  sshPubKeyRfc4716:
    /^---- BEGIN SSH2 PUBLIC KEY ----(\n|\r|\f)((.+)?((\n|\r|\f).+)*)(\n|\r|\f)---- END SSH2 PUBLIC KEY ----[\n\r\f]*$/,
  sshPubKeyOpenSsh:
    /^(sk-ssh-ed25519@openssh\.com|ssh-(rsa|ed25519)) AAAA[0-9A-Za-z+/]+[=]{0,3}\s*(.+)?$/,
  pgpPubKey:
    /^-----BEGIN PGP PUBLIC KEY BLOCK-----(\n|\r|\f)((.+)?((\n|\r|\f).+)*)(\n|\r|\f)-----END PGP PUBLIC KEY BLOCK-----[\n\r\f]*$/,
  netmask:
    /^(128|192|224|24[08]|25[245].0.0.0)|(255.(0|128|192|224|24[08]|25[245]).0.0)|(255.255.(0|128|192|224|24[08]|25[245]).0)|(255.255.255.(0|128|192|224|24[08]|252))$/,
  // See https://www.w3schools.com/Jsref/jsref_regexp_wordchar.asp
  wordChars: /^[\w]+$/,
  binaryUnit: /^\d+(.\d+)?\s?(b|[kmgtpezy]ib)$/i,
  macAddress: /^[a-fA-F0-9]{2}(:[a-fA-F0-9]{2}){5}$/
};

const isEmptyInputValue = (value: any): boolean => _.isNull(value) || value.length === 0;

const getControlName = (control: AbstractControl): string | undefined => {
  if (!control || !control.parent) {
    return undefined;
  }
  return _.keys(control.parent.controls).find(
    (key) => control === _.get(control.parent.controls, key)
  );
};

/**
 * Get the data on the top form.
 *
 * @param control The control to start searching for the top most form.
 * @return The raw values of the top form.
 */
const getFormValues = (control: AbstractControl): Array<any> => {
  if (!control) {
    return [];
  }
  let parent: FormGroup | FormArray | null = control.parent;
  while (parent?.parent) {
    parent = parent.parent;
  }
  return parent ? parent.getRawValue() : [];
};

export class CustomValidators {
  /**
   * Validator that requires the control have a non-empty value if the
   * specified constraint succeeds.
   *
   * @param constraint The constraint to process.
   * @returns A validator function that returns an error map with the
   *   `required` property if the validation constraint succeeds and
   *   the control's value is empty, otherwise `null`.
   */
  static requiredIf(constraint: Constraint): ValidatorFn {
    let hasSubscribed = false;
    // Determine the properties involved in the constraint.
    const props = ConstraintService.getProps(constraint);
    return (control: AbstractControl): ValidationErrors | null => {
      if (!control.parent) {
        return null;
      }
      // Ensure to not subscribe to changes of the own control.
      _.pull(props, getControlName(control));
      // Subscribe to value changes for all fields involved.
      if (!hasSubscribed) {
        props.forEach((path) => {
          control.parent
            .get(path)
            .valueChanges.pipe(debounceTime(5))
            .subscribe(() => {
              control.updateValueAndValidity({ emitEvent: false });
            });
        });
        hasSubscribed = true;
      }
      const fulfilled = ConstraintService.test(constraint, getFormValues(control));
      if (!fulfilled) {
        return null;
      }
      return isEmptyInputValue(control.value) ? { required: true } : null;
    };
  }

  /**
   * Validator that requires the specified constraint to be truthy.
   * The specified error code and data is applied if the constraint
   * is falsy.
   *
   * @param constraint The constraint to process.
   * @param errorCode The error code, e.g. 'required', 'maxLength',
   *   'min' or any code of an already existing validator.
   *   Defaults to 'constraint'.
   * @param context The form page context which contains the route
   *   configuration and params.
   * @param errorData The error data. For most of the existing
   *   validators this is a boolean `true`. But it can also contain
   *   an error message. In this case the message can be a tokenized
   *   string that will be formatted using the values from the form.
   * @returns A validator function that returns an error map with the
   *   specified error property if the validation constraint fails,
   *   otherwise `null`.
   */
  static constraint(
    constraint: Constraint,
    context: Record<string, any>,
    errorCode?: string,
    errorData?: any
  ): ValidatorFn {
    let hasSubscribed = false;
    // Determine the properties involved in the constraint.
    const props = ConstraintService.getProps(constraint);
    return (control: AbstractControl): ValidationErrors | null => {
      if (!control.parent) {
        return null;
      }
      // Ensure to not subscribe to changes of the own control.
      _.pull(props, getControlName(control));
      // Subscribe to value changes for all fields involved.
      if (!hasSubscribed) {
        props.forEach((path) => {
          control.parent
            .get(path)
            .valueChanges.pipe(debounceTime(5))
            .subscribe(() => {
              control.updateValueAndValidity({ emitEvent: false });
            });
        });
        hasSubscribed = true;
      }
      const values = _.merge({}, context, getFormValues(control));
      const fulfilled = ConstraintService.test(constraint, values);
      if (fulfilled) {
        return null;
      }
      const error = {};
      _.set(
        error,
        _.defaultTo(errorCode, 'constraint'),
        _.defaultTo(formatDeep(errorData, values), true)
      );
      return error;
    };
  }

  // eslint-disable-next-line @typescript-eslint/ban-types
  static pattern(pattern: string | RegExp | Function, errorData?: any): ValidatorFn {
    let test: (value: any) => boolean;
    if (_.isString(pattern)) {
      if (!_.startsWith(pattern, '^')) {
        pattern = `^${pattern}`;
      }
      if (!_.endsWith(pattern, '$')) {
        pattern += '$';
      }
      test = (value: any) => new RegExp(pattern as string).test(value);
    } else if (_.isRegExp(pattern)) {
      test = (value: any) => (pattern as RegExp).test(value);
    } else if (_.isFunction(pattern)) {
      test = pattern;
    } else {
      return Validators.nullValidator;
    }
    return (control: AbstractControl): ValidationErrors | null => {
      let value = control.value;
      // Don't validate empty values to allow optional controls.
      if (isEmptyInputValue(value)) {
        return null;
      }
      // Ensure that the value is a string unless the pattern is a
      // function, in which case it can take care of it itself.
      if (!_.isFunction(pattern) && !_.isString(value)) {
        value = _.toString(value);
      }
      return test(value)
        ? null
        : {
            pattern: _.defaultTo(
              errorData,
              format(gettext('The value does not match the pattern /{{ pattern }}/.'), {
                pattern: pattern.toString()
              })
            )
          };
    };
  }

  /**
   * Validator that requires the control's value to pass the specified test.
   *
   * @param name The name of the type to be tested, e.g. `username`,
   *   `email` or `ip`.
   * @returns A validator function that returns an error map with the
   *   `pattern` property if the validation check fails, otherwise `null`.
   */
  static patternType(name: string): ValidatorFn {
    switch (name) {
      case 'userName':
        return CustomValidators.pattern(regExp.userName, gettext('Invalid user name.'));
      case 'groupName':
        return CustomValidators.pattern(regExp.groupName, gettext('Invalid group name.'));
      case 'shareName':
        return CustomValidators.pattern(
          regExp.shareName,
          gettext(
            'This field contains invalid characters, e.g. a blank or "/[]:+|<>=;,*? character.'
          )
        );
      case 'email':
        return CustomValidators.pattern(
          (value) => validator.isEmail(value),
          gettext('This field should be an email address.')
        );
      case 'macAddress':
        return CustomValidators.pattern(
          regExp.macAddress,
          gettext('This field should be a MAC address, e.g. 00:80:41:ae:fd:7e.')
        );
      case 'ipv4':
        return CustomValidators.pattern(
          (value) => validator.isIP(value, 4),
          gettext('This field should be an IPv4 address.')
        );
      case 'ipv6':
        return CustomValidators.pattern(
          (value) => validator.isIP(value, 6),
          gettext('This field should be an IPv6 address.')
        );
      case 'ip':
        return CustomValidators.pattern(
          (value) => validator.isIP(value),
          gettext('This field should be an IP address.')
        );
      case 'ipList':
        return CustomValidators.pattern((value) => {
          const parts = _.split(value, /[,;]/);
          return parts.every((part) => {
            part = _.trim(part);
            return validator.isIP(part);
          });
        }, gettext('This field should be a list of IP addresses separated by <,> or <;>.'));
      case 'ipNetCidr':
        return CustomValidators.pattern(
          (value) => regExp.ipv4NetCidr.test(value) || regExp.ipv6NetCidr.test(value),
          gettext('This field should be a IP network address in CIDR notation.')
        );
      case 'hostName':
        return CustomValidators.pattern(regExp.hostName, gettext('Invalid host name.'));
      case 'hostNameIpNetCidr':
        return CustomValidators.pattern(
          (value) =>
            regExp.hostName.test(value) ||
            regExp.ipv4NetCidr.test(value) ||
            regExp.ipv6NetCidr.test(value),
          gettext('This field should be a host name or an IP address in CIDR notation.')
        );
      case 'hostNameIpNetCidrList':
        return CustomValidators.pattern((value) => {
          const parts = _.split(value, /[,;]/);
          return parts.every((part) => {
            part = _.trim(part);
            return (
              regExp.hostName.test(part) ||
              regExp.ipv4NetCidr.test(part) ||
              regExp.ipv6NetCidr.test(part)
            );
          });
        }, gettext('This field should be a list of host names or IP addresses in CIDR notation separated by <,> or <;>.'));
      case 'domainName':
        return CustomValidators.pattern(regExp.domainName, gettext('Invalid domain name.'));
      case 'domainNameList':
        return CustomValidators.pattern((value) => {
          const parts = _.split(value, /[,;]/);
          return parts.every((part) => {
            part = _.trim(part);
            return regExp.domainName.test(part);
          });
        }, gettext('This field should be a list of domain names or IP addresses separated by <,> or <;>.'));
      case 'domainNameIp':
        return CustomValidators.pattern(
          (value) => regExp.domainName.test(value) || validator.isIP(value),
          gettext('This field should be a domain name or an IP address.')
        );
      case 'domainNameIpList':
        return CustomValidators.pattern((value) => {
          const parts = _.split(value, /[,;]/);
          return parts.every((part) => {
            part = _.trim(part);
            return regExp.domainName.test(part) || validator.isIP(part);
          });
        }, gettext('This field should be a list of domain names or IP addresses.'));
      case 'netbiosName':
        return CustomValidators.pattern(regExp.netbiosName, gettext('Invalid NetBIOS name.'));
      case 'port':
        return CustomValidators.pattern(
          regExp.port,
          gettext('This field should be a network port in the range of 1 to 65535.')
        );
      case 'numeric':
      case 'integer': // deprecated
        return CustomValidators.pattern(
          regExp.numeric,
          gettext('This field should contain a numeric value.')
        );
      case 'decimal':
      case 'float': // deprecated
        return CustomValidators.pattern(
          regExp.decimal,
          gettext('This field should contain a decimal value.')
        );
      case 'time':
        return CustomValidators.pattern(
          regExp.time,
          gettext('This field must have the format <hh:mm:ss>.')
        );
      case 'sshPubKey':
        return CustomValidators.pattern(
          (value) => regExp.sshPubKeyOpenSsh.test(value) || regExp.sshPubKeyRfc4716.test(value),
          gettext('Invalid SSH public key (OpenSSH or RFC 4716 format)')
        );
      case 'sshPubKeyRfc4716':
        return CustomValidators.pattern(
          regExp.sshPubKeyRfc4716,
          gettext('Invalid SSH public key (RFC 4716 format).')
        );
      case 'sshPubKeyOpenSsh':
        return CustomValidators.pattern(
          regExp.sshPubKeyOpenSsh,
          gettext('Invalid SSH public key (OpenSSH format).')
        );
      case 'pgpPubKey':
        return CustomValidators.pattern(regExp.pgpPubKey, gettext('Invalid PGP public key.'));
      case 'netmask':
        return CustomValidators.pattern(
          regExp.netmask,
          gettext('This field should be a netmask within the range 128.0.0.0 - 255.255.255.252.')
        );
      case 'wordChars':
        return CustomValidators.pattern(
          regExp.wordChars,
          gettext(
            'This field contains invalid characters, only alphanumeric characters and underscore are allowed.'
          )
        );
      case 'binaryUnit':
        return CustomValidators.pattern(
          regExp.binaryUnit,
          gettext('This field should contain a number with a binary prefix.')
        );
    }
    throw new Error(`Unknown pattern ${name}!`);
    // return Validators.nullValidator;
  }

  static minBinaryUnit(min: number): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      if (isEmptyInputValue(control.value) || isEmptyInputValue(min)) {
        return null;
      }
      const value = toBytes(control.value);
      if (_.isString(value) || isEmptyInputValue(value)) {
        return null;
      }
      return !isNaN(value) && value < min ? { min: { min, actual: value } } : null;
    };
  }

  static maxBinaryUnit(max: number): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      if (isEmptyInputValue(control.value) || isEmptyInputValue(max)) {
        return null;
      }
      const value = toBytes(control.value);
      if (_.isString(value) || isEmptyInputValue(value)) {
        return null;
      }
      return !isNaN(value) && value > max ? { max: { max, actual: value } } : null;
    };
  }
}
