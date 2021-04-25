import { FormFieldConfig } from '~/app/core/components/limn-ui/models/form-field-config.type';
import { Constraint } from '~/app/shared/models/constraint.type';
import { ModalDialogConfig } from '~/app/shared/models/modal-dialog-config.type';
import { TaskDialogConfig } from '~/app/shared/models/task-dialog-config.type';

export type FormPageConfig = {
  // Specifies a unique ID for the form.
  id?: string;
  // A title within the header.
  title?: string;
  // A subtitle within the header.
  subTitle?: string;
  // An image used as an avatar within the header.
  icon?: string;
  request?: {
    // The name of the RPC service.
    service: string;
    // The name of the RPC to get the data.
    get?: {
      // The name of the RPC.
      method: string;
      // The RPC parameters. The given values can be tokenized
      // strings that will be formatted using the values from the
      // route parameters.
      // Example:
      // Configured route = '/usermgmt/users/edit/:name'
      // URL = '/usermgmt/users/edit/test01'
      // params = { username: '{{ name }}', foo: 2 } => { username: 'test01', foo: 2 }
      params?: Record<string, any>;
      // Set `true` if the RPC is a long running background task.
      task?: boolean;
      // Transform the given parameters in the request response.
      // The given parameters can be tokenized strings that will be
      // formatted using the request response. Finally these parameters
      // are merged with the request response.
      // Example:
      // Response = { foo: 'bar', num: '3' }
      // transform = { foo: '{{ foo }} xxx', num: '{{ num | int }}', add: 'foo' }
      // Result = { foo: 'bar xxx', num: 3, add: 'foo' }
      transform?: { [key: string]: string };
      // The names of the properties to be filtered. Depending on the
      // given 'mode' these properties are picked or omitted from the
      // request response. The resulting parameters are used to update
      // the form field values.
      filter?: {
        mode: 'pick' | 'omit';
        props: Array<string>;
      };
    };
    // The name of the RPC to update the data.
    post?: {
      // The name of the RPC.
      method: string;
      // The RPC parameters for the RPC request are taken fom the form
      // fields (name/value) except those with `submitValues=false`.
      // Additional parameters can be defined here. The given values can
      // be tokenized strings that will be formatted using the values
      // from the form fields.
      // Finally the form field values are overwritten and merged with
      // this given parameters. This way it is possible to modify the
      // values dynamically before they are submitted.
      params?: Record<string, any>;
      // If set to `true`, an array of unique values that are included
      // in the given RPC parameters and the form field values is used
      // by the RPC request. Defaults to `false`.
      intersectParams?: boolean;
      // Set `true` if the RPC is a long running background task.
      task?: boolean;
      // If set, a dialog is shown that must be confirmed before the
      // RPC is executed. The RPC is only executed when the dialog
      // result is `true`.
      confirmationDialogConfig?: ModalDialogConfig;
      // If a message is defined, then the UI will be blocked and
      // the message is displayed while the request is running.
      progressMessage?: string;
    };
  };
  // The configuration of the form field controls.
  fields: Array<FormFieldConfig>;
  // Form pages can have buttons in their footer.
  buttonAlign?: 'left' | 'center' | 'right';
  buttons?: Array<FormPageButtonConfig>;
};

export type FormPageButtonConfig = {
  // Specifies a template that pre-configures the button:
  // back   - A button with the text 'Back'.
  // cancel - A button with the text 'Cancel'.
  // submit - A button with the text 'Save'. It will trigger the
  //          configured form 'post' request. An additional 'url',
  //          'request' or 'click' action is executed afterwards.
  template?: 'back' | 'cancel' | 'submit';
  // The text displayed in the button.
  text?: string;
  // Custom CSS class.
  class?: string;
  // Disable the button. Defaults to 'false'.
  disabled?: boolean;
  // If set, the constraints must succeed to enable the button,
  // otherwise it is disabled.
  enabledConstraint?: Constraint;
  // If set, a dialog is shown that must be confirmed before the
  // button action is executed. The button action is only executed
  // when the dialog result is `true`.
  confirmationDialogConfig?: ModalDialogConfig;
  // The activity to be done when the button is pressed.
  execute?: FormPageButtonExecute;
};

export type FormPageButtonExecute = {
  type: 'url' | 'request' | 'taskDialog' | 'click';
  // Navigate to this URL when the button has been clicked.
  // The URL is formatted with the form field values and the page
  // context (please see AbstractPageComponent::pageContext).
  url?: string;
  // Execute the specified RPC when the button has been clicked.
  request?: {
    // The name of the RPC service.
    service: string;
    // The name of the RPC.
    method: string;
    // The RPC parameters. The value can be a tokenized string that
    // will be formatted using the form values.
    params?: Record<string, any>;
    // Set `true` if the RPC is a background task.
    task?: boolean;
    // If a message is defined, then the UI will be blocked and the
    // message is displayed while the request is running.
    progressMessage?: string;
    // Display a notification when the request was successful.
    successNotification?: string;
    // Navigate to this URL when the request was successful.
    // The URL is formatted with the form field values and the page
    // context (please see AbstractPageComponent::pageContext).
    // The RPC response is accessible via '_response'.
    // Example:
    // /foo/bar/{{ xyz }}/{{ _response['baz'] }}
    // where `xyz` will be replaced by the value of the form field
    // named `xyz` and `_response['baz']` by the property `baz` of
    // the map/object returned by the RPC.
    successUrl?: string;
  };
  // Display a dialog that shows the output of the given RPC.
  taskDialog?: {
    config: TaskDialogConfig;
    // Navigate to this URL after the dialog has been closed, but only
    // if the specified request was previously successfully completed.
    // The URL is formatted with the form field values and the page
    // context (please see AbstractPageComponent::pageContext).
    successUrl?: string;
  };
  // A callback function. Internal only.
  click?: (buttonConfig: FormPageButtonConfig, values: Record<string, any>) => void;
};
