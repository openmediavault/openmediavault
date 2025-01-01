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
import { DatatablePageConfig } from '~/app/core/components/intuition/models/datatable-page-config.type';
import { FormPageConfig } from '~/app/core/components/intuition/models/form-page-config.type';
import { SelectionListPageConfig } from '~/app/core/components/intuition/models/selection-list-page-config.type';
import { TextPageConfig } from '~/app/core/components/intuition/models/text-page-config.type';

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
