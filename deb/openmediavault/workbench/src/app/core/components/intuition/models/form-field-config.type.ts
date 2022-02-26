/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2022 Volker Theile
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
import { MatFormDatatableAction } from '~/app/core/components/intuition/material/mat-form-datatable/mat-form-datatable.component';
import { Constraint } from '~/app/shared/models/constraint.type';
import { DataStore } from '~/app/shared/models/data-store.type';
import { DatatableColumn } from '~/app/shared/models/datatable-column.type';
import { Sorter } from '~/app/shared/models/sorter.type';

export type FormFieldConfig = {
  // The following field controls are supported:
  // .--------------------------------------------------------------------.
  // | Type        | Description                                          |
  // |-------------|------------------------------------------------------|
  // | confObjUuid | This is a hidden field that contains an UUID. By     |
  // |             | default it is set to the UUID that tells the backend |
  // |             | that it should handle the the data to create a new   |
  // |             | database configuration object.                       |
  // | hidden      | A hidden form field.                                 |
  // | divider     | Draws a horizontal line.                             |
  // | paragraph   | Displays a title and draws a horizontal line.        |
  // | ...         | ...                                                  |
  // | container   | Align child fields in horizontal order.              |
  // '--------------------------------------------------------------------'
  type:
    | 'confObjUuid'
    | 'hidden'
    | 'divider'
    | 'paragraph'
    | 'button'
    | 'iconButton'
    | 'textInput'
    | 'folderBrowser'
    | 'numberInput'
    | 'binaryUnitInput'
    | 'checkbox'
    | 'textarea'
    | 'fileInput'
    | 'select'
    | 'sharedFolderSelect'
    | 'sshCertSelect'
    | 'sslCertSelect'
    | 'passwordInput'
    | 'datePicker'
    | 'datatable'
    | 'slider'
    | 'container';
  name?: string;
  label?: string;
  placeholder?: string;
  tooltip?: string;
  hint?: string;
  value?: any;
  readonly?: boolean;
  // Disable the field.
  // Use a tokenized string to be able to mark the field as disabled
  // dynamically based on its evaluation result on form initialization.
  // The special token `_routeConfig` can be to access the components
  // route configuration. Make sure the token will be evaluated to a
  // boolean value.
  //
  // Example:
  // { disabled: '{{ _routeConfig.data.editing | toboolean }}' }
  // The component's route configuration:
  // {
  //   path: 'edit/:name',
  //   component: UserFormPageComponent,
  //   data: {
  //     title: gettext('Edit'),
  //     editing: true,
  //     notificationTitle: gettext('Updated user "{{ name }}".')
  //   }
  // }
  disabled?: boolean | string;
  // Modify the form field depending on a specified constraint. The
  // constraint must be truthy to apply.
  modifiers?: Array<FormFieldModifier>;
  autofocus?: boolean;
  icon?: string;
  submitValue?: boolean;

  // The event name for control to update upon. Defaults to `change`.
  updateOn?: 'change' | 'blur';
  // The validators to ensure the form field content is
  // in a valid state.
  validators?: {
    // Mark the field as required.
    // Use a tokenized string to be able to mark the field as required
    // dynamically based on its evaluation result. The special token
    // `_routeConfig` can be to access the components route configuration.
    // Make sure the token will be evaluated to a boolean value.
    required?: boolean | string;
    // When the constraint succeeds and the control has
    // an empty value, then the 'required' error is set.
    requiredIf?: Constraint;
    minLength?: number;
    maxLength?: number;
    min?: number;
    max?: number;
    email?: boolean;
    custom?: Array<FormFieldConstraintValidator>;
    pattern?: {
      // The regular expression, e.g. '^\s+$'.
      // Note, if the value to be compared is an array, then it will
      // be converted automatically into a comma separated string.
      pattern: string;
      // The error data; normally the error message.
      errorData?: any;
    };
    patternType?:
      | 'userName'
      | 'groupName'
      | 'shareName'
      | 'email'
      | 'ip'
      | 'ipv4'
      | 'ipv6'
      | 'ipList'
      | 'ipNetCidr'
      | 'hostName'
      | 'hostNameIpNetCidr'
      | 'hostNameIpNetCidrList'
      | 'domainName'
      | 'domainNameList'
      | 'domainNameIp'
      | 'domainNameIpList'
      | 'port'
      | 'time'
      | 'numeric'
      | 'decimal'
      | 'integer' // deprecated
      | 'float' // deprecated
      | 'sshPubKey'
      | 'sshPubKeyRfc4716'
      | 'sshPubKeyOpenSsh'
      | 'netmask'
      | 'wordChars'
      | 'binaryUnit';
  };

  // --- textarea | textInput | fileInput | folderBrowser | password ---
  // Defaults to 'off'.
  // See https://developer.mozilla.org/en-US/docs/Web/HTML/Global_attributes/autocapitalize
  autocapitalize?: 'none' | 'sentences' | 'words' | 'characters';

  // --- container ---
  fields?: Array<FormFieldConfig>;
  // Fields in a container will respect the 'flex' configuration.
  // Specifies the size of the field in percent.
  flex?: number;

  // --- button | divider ---
  text?: string;

  // --- button | iconButton ---
  click?: () => void;
  request?: {
    // The name of the RPC service.
    service: string;
    // The name of the RPC.
    method: string;
    // The RPC parameters. The parameters will be formatted using
    // the values from the parent form.
    params?: Record<string, any>;
    // Set `true` if the RPC is a background task.
    task?: boolean;
    // If a message is defined, then the UI will be blocked and
    // the message is displayed while the request is running.
    progressMessage?: string;
    // Display a notification when the request was successful.
    successNotification?: string;
    // Navigate to this URL when the request was successful.
    // The URL will be formatted using the values from the parent
    // form. The RPC response is accessible via '_response'.
    // Example:
    // /foo/bar/{{ xyz }}/{{ _response['baz'] }}
    // where `xyz` will be replaced by the value of the form field
    // named `xyz` and `_response['baz']` by the property `baz` of
    // the map/object returned by the RPC.
    // Example:
    // /externalRedirect/{{ _response['url'] }}
    // Redirect to an external URL. The URL must not be escaped,
    // this will be done automatically.
    successUrl?: string;
  };
  // The URL will be formatted using the parent form field values.
  url?: string;

  // --- folderBrowser ---
  dirType?: 'sharedfolder' | 'mntent';
  // The name of the field that contains the UUID of the
  // shared folder or mount point configuration object.
  dirRefIdField?: string;
  // Set to `true` to show the path of the database object
  // specified with `dirType` and `dirRefIdField`.
  // Defaults to `false`.
  dirVisible?: boolean;

  // --- numberInput | slider ---
  step?: number;

  // --- numberInput | password | textInput | binaryUnitInput ---
  // See https://developer.mozilla.org/en-US/docs/Web/HTML/Attributes/autocomplete.
  autocomplete?: string;
  // Note, this button is only visible if the browser supports
  // that. The following requirements must be met:
  // - The HTTPS protocol is used. localhost is also supported.
  // - The site is not embedded in an iFrame.
  hasCopyToClipboardButton?: boolean;

  // --- binaryUnitInput ---
  // The default unit used when the value does not contain a unit.
  // Defaults to bytes (`B`).
  defaultUnit?: 'B' | 'KiB' | 'MiB' | 'GiB' | 'TiB' | 'PiB' | 'EiB' | 'ZiB' | 'YiB';
  // The number of digits after the decimal point.
  // Defaults to `0`.
  fractionDigits?: number;

  // --- textarea | fileInput ---
  cols?: number;
  // Defaults to 4.
  rows?: number;
  // Defaults to 'soft'.
  wrap?: 'hard' | 'soft' | 'off';

  // --- textarea | textInput | fileInput | folderBrowser ---
  // Use a monospace font.
  monospace?: boolean;

  // --- fileInput ---
  // Takes a comma-separated list of one or more file types, or
  // unique file type specifiers, describing which file types to
  // allow.
  accept?: string;
  // Remove whitespace from both ends of the file content after
  // loading, this includes space, tab, no-break space and all the
  // line terminator characters (e.g. LF, CR).
  // Defaults to `true` if the `rows` property is set to 1,
  // otherwise `false`.
  trim?: boolean;

  // --- select ---
  multiple?: boolean;
  // Defaults to `value`.
  valueField?: string;
  // Defaults to `text`.
  textField?: string;
  store?: DataStore;
  // Add an empty option to be able to clear the selection.
  // Defaults to `false`.
  hasEmptyOption?: boolean;
  // The text displayed in the option with the empty value.
  // Defaults to `None`.
  emptyOptionText?: string;
  selectionChange?: (value: any) => void;

  // --- divider ---
  title?: string;

  // --- datatable ---
  columns?: Array<DatatableColumn>;
  columnMode?: 'standard' | 'flex' | 'force';
  hasActionBar?: boolean;
  hasSearchField?: boolean;
  hasHeader?: boolean;
  hasFooter?: boolean;
  selectionType?: 'none' | 'single' | 'multi';
  limit?: number;
  sorters?: Array<Sorter>;
  actions?: Array<MatFormDatatableAction>;
  // Specifies the type of the array items. Defaults to `object`.
  valueType?: 'string' | 'integer' | 'number' | 'object';
};

export type FormFieldConstraintValidator = {
  // When the constraint is falsy, then the specified
  // error code is set.
  constraint: Constraint;
  // The error code, e.g. 'required' or 'email'.
  // Defaults to 'constraint'.
  errorCode?: string;
  // The error data, e.g. a boolean `true` or the message displayed
  // below the form field.
  errorData?: any;
};

export type FormFieldModifier = {
  type:
    | 'disabled'
    | 'enabled'
    | 'checked'
    | 'unchecked'
    | 'focused'
    | 'visible'
    | 'hidden'
    | 'value';
  // Optional configuration used by modifiers. This is required by
  // the 'value' modifier, e.g. '{{ <NAME> }}' to set the value
  // of the given field.
  typeConfig?: any;
  // Apply the opposite type, e.g. `disabled` for `enabled`, if the
  // constraint is falsy. Defaults to `true`.
  opposite?: boolean;
  // The constraint can access the current form field
  // values, e.g. '{ field: '<NAME>' }'
  constraint: Constraint;
};
