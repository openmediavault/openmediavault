import { AbstractControl, FormControl, FormGroup, ValidatorFn } from '@angular/forms';

import { CustomValidators } from '~/app/shared/forms/custom-validators';

describe('CustomValidators', () => {
  const formControl = new FormControl();

  describe('requiredIf', () => {
    let formGroup: FormGroup;
    let emailControl: AbstractControl;
    let enabledControl: AbstractControl;
    const validator = CustomValidators.requiredIf({
      operator: 'eq',
      arg0: { prop: 'enabled' },
      arg1: true
    });

    beforeEach(() => {
      formGroup = new FormGroup({
        enabled: new FormControl(false),
        email: new FormControl('')
      });
      emailControl = formGroup.get('email');
      enabledControl = formGroup.get('enabled');
    });

    it('should not validate requiredIf (1)', () => {
      expect(validator(emailControl)).toBeNull();
    });

    it('should not validate requiredIf (2)', () => {
      emailControl.setValue('foo@bar.com');
      expect(validator(emailControl)).toBeNull();
    });

    it('should not validate requiredIf (3)', () => {
      enabledControl.setValue(true);
      emailControl.setValue('foo@bar.com');
      expect(validator(emailControl)).toBeNull();
    });

    it('should validate requiredIf', () => {
      enabledControl.setValue(true);
      expect(validator(emailControl)).toEqual({ required: true });
    });
  });

  describe('constraint', () => {
    let formGroup: FormGroup;
    let passwordControl: AbstractControl;
    let passwordConfControl: AbstractControl;
    const validator = CustomValidators.constraint(
      {
        operator: 'eq',
        arg0: { prop: 'password' },
        arg1: { prop: 'passwordconf' }
      },
      undefined,
      'constraint',
      'Password doesn\'t match.'
    );

    beforeEach(() => {
      formGroup = new FormGroup({
        password: new FormControl(''),
        passwordconf: new FormControl('')
      });
      passwordControl = formGroup.get('password');
      passwordConfControl = formGroup.get('passwordconf');
    });

    it('should not validate constraint (1)', () => {
      expect(validator(passwordConfControl)).toBeNull();
    });

    it('should not validate constraint (2)', () => {
      passwordControl.setValue('foo');
      passwordConfControl.setValue('foo');
      expect(validator(passwordConfControl)).toBeNull();
    });

    it('should validate constraint (1)', () => {
      passwordControl.setValue('foo');
      expect(validator(passwordConfControl)).toEqual({ constraint: 'Password doesn\'t match.' });
    });

    it('should validate constraint (2)', () => {
      passwordConfControl.setValue('bar');
      expect(validator(passwordConfControl)).toEqual({ constraint: 'Password doesn\'t match.' });
    });

    it('should validate constraint (3)', () => {
      passwordControl.setValue('foo');
      passwordConfControl.setValue('bar');
      expect(validator(passwordConfControl)).toEqual({ constraint: 'Password doesn\'t match.' });
    });
  });

  describe('patternType', () => {
    describe('email', () => {
      let validator: ValidatorFn;

      beforeEach(() => {
        validator = CustomValidators.patternType('email');
      });

      it('should validate email', () => {
        formControl.setValue('foo@bar.com');
        expect(validator(formControl)).toBeNull();
      });

      it('should not validate email', () => {
        formControl.setValue('bar.com');
        expect(validator(formControl)).toEqual({
          pattern: 'This field should be an email address.'
        });
      });
    });

    describe('ip', () => {
      let validator: ValidatorFn;

      beforeEach(() => {
        validator = CustomValidators.patternType('ip');
      });

      it('should validate IP', () => {
        formControl.setValue('172.16.0.12');
        expect(validator(formControl)).toBeNull();
      });

      it('should not validate IP', () => {
        formControl.setValue('bar');
        expect(validator(formControl)).toEqual({ pattern: 'This field should be an IP address.' });
      });
    });

    describe('ipv4', () => {
      let validator: ValidatorFn;

      beforeEach(() => {
        validator = CustomValidators.patternType('ipv4');
      });

      it('should validate IPv4', () => {
        formControl.setValue('8.8.8.8');
        expect(validator(formControl)).toBeNull();
      });

      it('should not validate IPv4 (1)', () => {
        formControl.setValue('bar');
        expect(validator(formControl)).toEqual({
          pattern: 'This field should be an IPv4 address.'
        });
      });

      it('should not validate IPv4 (2)', () => {
        formControl.setValue('2001:0db8:85a3:0000:0000:8a2e:0370:7334');
        expect(validator(formControl)).toEqual({
          pattern: 'This field should be an IPv4 address.'
        });
      });

      it('should not validate IPv4 (3)', () => {
        formControl.setValue('192.168.0');
        expect(validator(formControl)).toEqual({
          pattern: 'This field should be an IPv4 address.'
        });
      });
    });

    describe('ipv6', () => {
      let validator: ValidatorFn;

      beforeEach(() => {
        validator = CustomValidators.patternType('ipv6');
      });

      it('should validate IPv6', () => {
        formControl.setValue('2001:0db8:85a3:0000:0000:8a2e:0370:7334');
        expect(validator(formControl)).toBeNull();
      });

      it('should not validate IPv6 (1)', () => {
        formControl.setValue('bar');
        expect(validator(formControl)).toEqual({
          pattern: 'This field should be an IPv6 address.'
        });
      });

      it('should not validate IPv6 (2)', () => {
        formControl.setValue('fd77:da36:::');
        expect(validator(formControl)).toEqual({
          pattern: 'This field should be an IPv6 address.'
        });
      });

      it('should not validate IPv6 (3)', () => {
        formControl.setValue('192.168.0.1');
        expect(validator(formControl)).toEqual({
          pattern: 'This field should be an IPv6 address.'
        });
      });
    });

    describe('domainName', () => {
      let validator: ValidatorFn;

      beforeEach(() => {
        validator = CustomValidators.patternType('domainName');
      });

      it('should validate domainName', () => {
        formControl.setValue('openmediavault.org');
        expect(validator(formControl)).toBeNull();
      });

      it('should not validate domainName', () => {
        formControl.setValue('45f+565-.');
        expect(validator(formControl)).toEqual({ pattern: 'Invalid domain name.' });
      });
    });

    describe('domainNameList', () => {
      let validator: ValidatorFn;

      beforeEach(() => {
        validator = CustomValidators.patternType('domainNameList');
      });

      it('should validate domainNameList (1)', () => {
        formControl.setValue('openmediavault.org');
        expect(validator(formControl)).toBeNull();
      });

      it('should validate domainNameList (2)', () => {
        formControl.setValue('openmediavault.org;foo.bar , xyz.abc');
        expect(validator(formControl)).toBeNull();
      });

      it('should not validate domainNameList', () => {
        formControl.setValue('45f+565-.');
        expect(validator(formControl)).toEqual({
          pattern:
            'This field should be a list of domain names or IP addresses separated by <,> or <;>.'
        });
      });
    });

    describe('domainNameIp', () => {
      let validator: ValidatorFn;

      beforeEach(() => {
        validator = CustomValidators.patternType('domainNameIp');
      });

      it('should validate domainNameIp (1)', () => {
        formControl.setValue('openmediavault.org');
        expect(validator(formControl)).toBeNull();
      });

      it('should validate domainNameIp (2)', () => {
        formControl.setValue('8.8.8.8');
        expect(validator(formControl)).toBeNull();
      });

      it('should not validate domainNameIp', () => {
        formControl.setValue('45f+565-.');
        expect(validator(formControl)).toEqual({
          pattern: 'This field should be a domain name or an IP address.'
        });
      });
    });

    describe('domainNameIpList', () => {
      let validator: ValidatorFn;

      beforeEach(() => {
        validator = CustomValidators.patternType('domainNameIpList');
      });

      it('should validate domainNameIpList (1)', () => {
        formControl.setValue('172.16.32.2');
        expect(validator(formControl)).toBeNull();
      });

      it('should validate domainNameIpList (2)', () => {
        formControl.setValue(' openmediavault.org; 172.16.32.2 , xyz.abc');
        expect(validator(formControl)).toBeNull();
      });

      it('should not validate domainNameIpList', () => {
        formControl.setValue('45f+565-.');
        expect(validator(formControl)).toEqual({
          pattern: 'This field should be a list of domain names or IP addresses.'
        });
      });
    });

    describe('integer', () => {
      let validator: ValidatorFn;

      beforeEach(() => {
        validator = CustomValidators.patternType('integer');
      });

      it('should validate integer [1]', () => {
        formControl.setValue(10);
        expect(validator(formControl)).toBeNull();
      });

      it('should validate integer [2]', () => {
        formControl.setValue(-5);
        expect(validator(formControl)).toBeNull();
      });

      it('should not validate integer [1]', () => {
        formControl.setValue(1.2);
        expect(validator(formControl)).toEqual({
          pattern: 'This field should contain an integer value.'
        });
      });

      it('should not validate integer [2]', () => {
        formControl.setValue('a');
        expect(validator(formControl)).toEqual({
          pattern: 'This field should contain an integer value.'
        });
      });
    });

    describe('float', () => {
      let validator: ValidatorFn;

      beforeEach(() => {
        validator = CustomValidators.patternType('float');
      });

      it('should validate float [1]', () => {
        formControl.setValue(10.1245);
        expect(validator(formControl)).toBeNull();
      });

      it('should validate float [2]', () => {
        formControl.setValue(-5.5);
        expect(validator(formControl)).toBeNull();
      });

      it('should validate float [3]', () => {
        formControl.setValue(10);
        expect(validator(formControl)).toBeNull();
      });

      it('should validate float [4]', () => {
        formControl.setValue(10.0);
        expect(validator(formControl)).toBeNull();
      });

      it('should not validate float', () => {
        formControl.setValue('a');
        expect(validator(formControl)).toEqual({
          pattern: 'This field should contain a floating point value.'
        });
      });
    });

    describe('binaryUnit', () => {
      let validator: ValidatorFn;

      beforeEach(() => {
        validator = CustomValidators.patternType('binaryUnit');
      });

      it('should validate binaryUnit [1]', () => {
        formControl.setValue('5B');
        expect(validator(formControl)).toBeNull();
      });

      it('should validate binaryUnit [2]', () => {
        formControl.setValue('2 tib');
        expect(validator(formControl)).toBeNull();
      });

      it('should validate binaryUnit [3]', () => {
        formControl.setValue(10);
        expect(validator(formControl)).toEqual({
          pattern: 'This field should contain a number with a binary prefix.'
        });
      });

      it('should validate binaryUnit [4]', () => {
        formControl.setValue('2  GiB');
        expect(validator(formControl)).toEqual({
          pattern: 'This field should contain a number with a binary prefix.'
        });
      });

      it('should validate binaryUnit [5]', () => {
        formControl.setValue('abc KiB');
        expect(validator(formControl)).toEqual({
          pattern: 'This field should contain a number with a binary prefix.'
        });
      });
    });

    describe('minBinaryUnit', () => {
      let validator: ValidatorFn;

      beforeEach(() => {
        validator = CustomValidators.minBinaryUnit(1048576); // 1 MiB
      });

      it('should validate minBinaryUnit [1]', () => {
        formControl.setValue('5 B');
        expect(validator(formControl)).toEqual({ min: { actual: 5, min: 1048576 } });
      });

      it('should validate minBinaryUnit [2]', () => {
        formControl.setValue('2 TiB');
        expect(validator(formControl)).toBeNull();
      });
    });

    describe('maxBinaryUnit', () => {
      let validator: ValidatorFn;

      beforeEach(() => {
        validator = CustomValidators.maxBinaryUnit(1048576); // 1 MiB
      });

      it('should validate maxBinaryUnit [1]', () => {
        formControl.setValue('5 KiB');
        expect(validator(formControl)).toBeNull();
      });

      it('should validate maxBinaryUnit [2]', () => {
        formControl.setValue('2 TiB');
        expect(validator(formControl)).toEqual({ max: { actual: 2199023255552, max: 1048576 } });
      });
    });
  });
});
