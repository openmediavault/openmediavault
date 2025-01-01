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
import { FormFieldConfig } from '~/app/core/components/intuition/models/form-field-config.type';
import { TaskDialogConfig } from '~/app/shared/models/task-dialog-config.type';

export type FormDialogConfig = {
  // Specifies a unique ID for the form.
  id?: string;
  // A title within the header.
  title?: string;
  // A subtitle within the header.
  subTitle?: string;
  // An image displayed within the header.
  icon?: string;
  // Width of the dialog in 'px' or '%'. Defaults to '50%'.
  width?: string;
  // The configuration of the form field controls.
  fields: Array<FormFieldConfig>;
  // The button configurations.
  buttons?: {
    submit?: FormDialogButtonConfig;
    cancel?: FormDialogButtonConfig;
  };
};

export type FormDialogButtonConfig = {
  // The text displayed in the button.
  text?: string;
  // Is the button hidden?
  hidden?: boolean;
  // The dialog close input. This can be a boolean, string or
  // anything else.
  dialogResult?: any;
  // The activity to be done when the button is pressed.
  execute?: FormDialogButtonExecute;
};

export type FormDialogButtonExecute = {
  type: 'url' | 'request' | 'taskDialog';
  // The URL of the route to be navigated to. This URL can
  // contain tokens that are replaced by the properties of
  // the selected row.
  // Example: /certificate/ssh/{{ uuid }}
  url?: string;
  // Execute the specified RPC.
  request?: {
    // The name of the RPC service.
    service: string;
    // The name of the RPC.
    method: string;
    // The RPC parameters for the RPC request are taken fom the form
    // fields (name/value) except those with `submitValues=false`.
    // Additional parameters can be defined here. The given values can
    // be tokenized strings that will be formatted using the values
    // from the form fields.
    // Finally, the form field values are overwritten and merged with
    // this given parameters. This way it is possible to modify the
    // values dynamically before they are submitted.
    params?: Record<string, any>;
    // Intersect the given RPC parameters and the form field values?
    // Defaults to `false`.
    intersectParams?: boolean;
    // Set `true` if the RPC is a background task.
    task?: boolean;
    // If a message is defined, then the UI will be blocked
    // and the message is displayed while the request is
    // executed.
    progressMessage?: string;
    // Display a notification when the request was successful.
    successNotification?: string;
    // Navigate to this URL when the request was successful.
    // The URL will be formatted with the form field values.
    successUrl?: string;
  };
  // Display a dialog that shows the output the given RPC.
  taskDialog?: {
    config: TaskDialogConfig;
    // Navigate to this URL after the dialog has been closed, but only
    // if the specified request was previously successfully completed.
    // The URL is formatted with the page context (please see
    // AbstractPageComponent::pageContext).
    successUrl?: string;
  };
};
