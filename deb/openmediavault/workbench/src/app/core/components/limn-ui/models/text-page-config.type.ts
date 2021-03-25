export type TextPageConfig = {
  title?: string;
  subTitle?: string;
  // The frequency in milliseconds in which the data should be reloaded.
  autoReload?: boolean | number;
  hasReloadButton?: boolean;
  hasCopyToClipboardButton?: boolean;
  request?: {
    // The name of the RPC service.
    service: string;
    // The name of the RPC to get the data.
    get?: {
      // The name of the RPC.
      method: string;
      // The RPC parameters. The value can be a
      // tokenized string that will be formatted using
      // the values from the route parameters.
      // Example:
      // Configured route = '/system/certificate/ssl/detail/:foo'
      // URL = '/system/certificate/ssl/detail/abc'
      // params = { bar: '{{ foo }}' } => { bar: 'abc' }
      params?: Record<string, any>;
      // Set `true` if the RPC is a long running background task.
      task?: boolean;
    };
  };
  buttonAlign?: 'left' | 'center' | 'right';
  buttons?: Array<TextPageButtonConfig>;
};

export type TextPageButtonConfig = {
  // Specifies a template that pre-configures the button:
  // back   - A button with the text 'Back'.
  // cancel - A button with the text 'Cancel'.
  template?: 'back' | 'cancel';
  // The text displayed in the button.
  text?: string;
  // Custom CSS class.
  class?: string;
  // The URL of the route to navigate to when the button has been
  // clicked.
  // Both options are mutually exclusive.
  url?: string;
  // A callback function that is called when the button has been
  // clicked. Internal only.
  click?: () => void;
};
