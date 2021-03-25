import { DatatablePageConfig } from '~/app/core/components/limn-ui/models/datatable-page-config.type';
import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';
import { SelectionListPageConfig } from '~/app/core/components/limn-ui/models/selection-list-page-config.type';
import { TextPageConfig } from '~/app/core/components/limn-ui/models/text-page-config.type';

export type TabsPageConfig = {
  // The tab page switches between views within a single route. Set
  // to `false` to navigate between multiple routes. Please refer to
  // https://material.angular.io/components/tabs/overview#tabs-and-navigation
  // for more information. Defaults to `true`.
  singleRoute?: boolean;
  tabs: Array<TabPageConfig>;
};

export type TabPageConfig = {
  label: string;

  // The following options have to be used when the tab page is used
  // to switch between views within a single route.
  type?: 'form' | 'datatable' | 'text' | 'selectionlist';
  config?: DatatablePageConfig | FormPageConfig | TextPageConfig | SelectionListPageConfig;

  // The following options have to be used when the tab page is used
  // to navigate between multiple routes.
  url?: string;
};
