import { FormDialogConfig } from '~/app/core/components/limn-ui/models/form-dialog-config.type';
import { DatatableActionConfig } from '~/app/shared/models/datatable-action-config.type';
import { ModalDialogConfig } from '~/app/shared/models/modal-dialog-config.type';
import { TaskDialogConfig } from '~/app/shared/models/task-dialog-config.type';

export type DatatablePageActionConfig = DatatableActionConfig & {
  // Specifies a template that pre-configures the datatable action:
  // add    - A button with an icon and the tooltip 'Add'.
  //          The button is always enabled.
  // create - A button with an icon and the tooltip 'Create'.
  //          The button is always enabled.
  // edit   - A button with an icon and the tooltip 'Edit'.
  //          One datatable row must be selected to enable the button.
  // delete - A button with an icon and the tooltip 'Delete'.
  //          At least one datatable row must be selected to enable
  //          the button.
  //          A confirmation dialog will be opened to confirm the
  //          deletion of the selected row(s).
  template?: 'add' | 'create' | 'edit' | 'delete';
  // If set, a dialog is shown that must be confirmed before the
  // configured action is executed. The datatable action is only
  // executed when the dialog result is `true`.
  confirmationDialogConfig?: ModalDialogConfig;
  // The activity to be done when the datatable action is executed.
  execute?: DatatablePageActionExecute;

  // --- menu ---
  // The actions displayed in the menu dropdown.
  actions?: Array<DatatablePageActionConfig>;
};

export type DatatablePageActionExecute = {
  // .-------------------------------------------------.
  // |            | single selection | multi selection |
  // |------------|------------------|-----------------|
  // | url        |        x         |                 |
  // | request    |        x         |       x         |
  // | taskDialog |        x         |                 |
  // | formDialog |        x         |                 |
  // '-------------------------------------------------'
  type: 'url' | 'request' | 'taskDialog' | 'formDialog';
  // An URL can contain "interpolate" delimiters that are
  // interpolated with the properties of the selected row.
  // Example: /certificate/ssh/{{ uuid }}
  url?: string;
  // Execute the specified RPC. The datatable will be
  // automatically reloaded on success.
  request?: {
    // The name of the RPC service.
    service: string;
    // The name of the RPC.
    method: string;
    // The RPC parameters. The value can be a
    // tokenized string that will be formatted using
    // the values from the processed datatable row.
    params?: Record<string, any>;
    // Set `true` if the RPC is a background task.
    task?: boolean;
    // If a message is defined, then the UI will be blocked and
    // the message is displayed while the request is running.
    progressMessage?: string;
    // Display a notification when the request was successful.
    // The notification can contain route config/params tokens.
    successNotification?: string;
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
  // Display a dialog with the specified form fields.
  formDialog?: FormDialogConfig;
};
