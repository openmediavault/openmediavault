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
import { MediaMatcher } from '@angular/cdk/layout';
import { Component, OnDestroy, OnInit } from '@angular/core';
import * as _ from 'lodash';

import { AbstractFormFieldComponent } from '~/app/core/components/intuition/form/components/abstract-form-field-component';

@Component({
  selector: 'omv-form-code-editor',
  templateUrl: './form-code-editor.component.html',
  styleUrls: ['./form-code-editor.component.scss']
})
export class FormCodeEditorComponent
  extends AbstractFormFieldComponent
  implements OnInit, OnDestroy
{
  options: Record<string, any> = {};

  private mediaQueryList: MediaQueryList;

  constructor(private mediaMatcher: MediaMatcher) {
    super();
  }

  ngOnInit(): void {
    super.ngOnInit();
    this.mediaQueryList = this.mediaMatcher.matchMedia('(prefers-color-scheme: light)');
    this.mediaQueryList.onchange = (event: MediaQueryListEvent) => {
      // Force an Angular change-detection.
      this.options = _.merge({}, this.options, {
        theme: this.getTheme(event)
      });
    };
    this.options = {
      readOnly: this.config.readonly,
      lineNumbers: this.config.lineNumbers ? 'on' : 'off',
      theme: this.getTheme(this.mediaQueryList),
      language: this.config.language,
      minimap: { enabled: false },
      scrollBeyondLastLine: false,
      automaticLayout: true
    };
  }

  ngOnDestroy(): void {
    this.mediaQueryList.onchange = undefined;
  }

  private getTheme(event: MediaQueryList | MediaQueryListEvent): string {
    return event.matches ? 'vs-light' : 'vs-dark';
  }
}
