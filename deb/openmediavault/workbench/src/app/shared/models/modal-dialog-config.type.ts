export type ModalDialogConfig = {
  // Specifies a template which pre-configures the dialog.
  // confirmation        - Displays 'Yes' and 'No' buttons.
  // confirmation-danger - Displays 'Yes' and 'No' buttons.
  //                       The 'Yes' button is marked in red.
  // information         - Displays a 'OK' button.
  template?: 'confirmation' | 'confirmation-danger' | 'information';
  title?: string;
  message: string;
  icon?: string;
  buttons?: Array<ModalDialogButtonConfig>;
};

export type ModalDialogButtonConfig = {
  // The text displayed in the button.
  text?: string;
  // Custom CSS class.
  class?: string;
  // Is the button disabled?
  disabled?: boolean;
  // Is the button auto-focused?
  autofocus?: boolean;
  // The dialog close input. This can be a boolean, string or
  // anything else.
  dialogResult?: any;
};
