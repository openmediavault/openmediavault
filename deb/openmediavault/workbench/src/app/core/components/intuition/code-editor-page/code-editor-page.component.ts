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
import { HttpErrorResponse } from '@angular/common/http';
import { AfterViewInit, Component, ElementRef, Inject, OnInit, ViewChild } from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';
import {
  autocompletion,
  closeBrackets,
  closeBracketsKeymap,
  completionKeymap
} from '@codemirror/autocomplete';
import { defaultKeymap, history, historyKeymap } from '@codemirror/commands';
import { json } from '@codemirror/lang-json';
import { python } from '@codemirror/lang-python';
import { xml } from '@codemirror/lang-xml';
import { yaml } from '@codemirror/lang-yaml';
import {
  bracketMatching,
  defaultHighlightStyle,
  foldGutter,
  foldKeymap,
  indentOnInput,
  StreamLanguage,
  syntaxHighlighting
} from '@codemirror/language';
import { shell } from '@codemirror/legacy-modes/mode/shell';
import { highlightSelectionMatches, searchKeymap } from '@codemirror/search';
import { EditorState, Extension, StateEffect } from '@codemirror/state';
import { oneDark } from '@codemirror/theme-one-dark';
import {
  EditorView,
  highlightActiveLine,
  highlightActiveLineGutter,
  keymap,
  lineNumbers
} from '@codemirror/view';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { EMPTY, Subscription, timer } from 'rxjs';
import { catchError, finalize } from 'rxjs/operators';

import { AbstractPageComponent } from '~/app/core/components/intuition/abstract-page-component';
import {
  CodeEditorPageButtonConfig,
  CodeEditorPageConfig
} from '~/app/core/components/intuition/models/code-editor-page-config.type';
import { Unsubscribe } from '~/app/decorators';
import { Icon } from '~/app/shared/enum/icon.enum';
import { RpcObjectResponse } from '~/app/shared/models/rpc.model';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { ClipboardService } from '~/app/shared/services/clipboard.service';
import {
  PrefersColorScheme,
  PrefersColorSchemeService
} from '~/app/shared/services/prefers-color-scheme.service';
import { RpcService } from '~/app/shared/services/rpc.service';

/**
 * Display text in a read-only textarea using a non-proportional font.
 */
@Component({
  selector: 'omv-intuition-code-editor-page',
  templateUrl: './code-editor-page.component.html',
  styleUrls: ['./code-editor-page.component.scss']
})
export class CodeEditorPageComponent
  extends AbstractPageComponent<CodeEditorPageConfig>
  implements OnInit, AfterViewInit
{
  @ViewChild('editorContainer', { static: true })
  _editorContainer: ElementRef;

  @Unsubscribe()
  private subscriptions: Subscription = new Subscription();

  public error: HttpErrorResponse;
  public icon = Icon;
  public loading = false;

  private _editorState: EditorState;
  private _editorView: EditorView;
  private _useDarkTheme = false;

  constructor(
    @Inject(ActivatedRoute) activatedRoute: ActivatedRoute,
    @Inject(AuthSessionService) authSessionService: AuthSessionService,
    @Inject(Router) router: Router,
    private clipboardService: ClipboardService,
    private prefersColorSchemeService: PrefersColorSchemeService,
    private rpcService: RpcService
  ) {
    super(activatedRoute, authSessionService, router);
  }

  override ngOnInit(): void {
    super.ngOnInit();
    this.subscriptions.add(
      timer(
        0,
        _.isNumber(this.config.autoReload) ? (this.config.autoReload as number) : null
      ).subscribe(() => {
        this.loadData();
      })
    );
    this.subscriptions.add(
      this.prefersColorSchemeService.change$.subscribe(
        (prefersColorScheme: PrefersColorScheme): void => {
          this._useDarkTheme = prefersColorScheme === 'dark';
          this._editorView.dispatch({
            effects: StateEffect.reconfigure.of(this.getExtensions())
          });
        }
      )
    );
  }

  override ngAfterViewInit(): void {
    this.createEditor();
  }

  onCopyToClipboard() {
    const content = this._editorView.state.doc.toString();
    this.clipboardService.copy(content);
  }

  onButtonClick(buttonConfig: CodeEditorPageButtonConfig) {
    if (_.isFunction(buttonConfig.click)) {
      buttonConfig.click();
    } else {
      this.router.navigateByUrl(buttonConfig.url);
    }
  }

  loadData() {
    const request = this.config.request;
    if (_.isPlainObject(request) && _.isString(request.service) && _.isPlainObject(request.get)) {
      this.loading = true;
      // noinspection DuplicatedCode
      this.rpcService[request.get.task ? 'requestTask' : 'request'](
        request.service,
        request.get.method,
        request.get.params
      )
        .pipe(
          catchError((error) => {
            this.error = error;
            return EMPTY;
          }),
          finalize(() => {
            this.loading = false;
          })
        )
        .subscribe((res: any) => {
          if (_.isString(request.get.format) && RpcObjectResponse.isType(res)) {
            res = RpcObjectResponse.format(request.get.format, res);
          }
          const state: EditorState = this._editorView.state;
          const transaction = state.update({
            changes: { from: 0, to: state.doc.length, insert: res }
          });
          this._editorView.dispatch(transaction);
        });
    }
  }

  protected override sanitizeConfig() {
    _.defaultsDeep(this.config, {
      autoReload: false,
      hasReloadButton: false,
      hasCopyToClipboardButton: false,
      language: 'none',
      lineNumbers: true,
      buttonAlign: 'end',
      buttons: []
    });
    // Set the default values of the buttons.
    _.forEach(this.config.buttons, (button) => {
      const template = _.get(button, 'template');
      switch (template) {
        case 'back':
          _.defaultsDeep(button, {
            text: gettext('Back')
          });
          break;
        case 'cancel':
          _.defaultsDeep(button, {
            text: gettext('Cancel')
          });
          break;
      }
    });
  }

  protected override onRouteParams() {
    // Format tokenized configuration properties.
    this.formatConfig(['title', 'subTitle', 'request.get.method', 'request.get.params']);
  }

  private createEditor(): void {
    this._editorState = EditorState.create({
      doc: '',
      extensions: this.getExtensions()
    });
    this._editorView = new EditorView({
      parent: this._editorContainer.nativeElement,
      state: this._editorState
    });
  }

  private getExtensions(): Extension {
    return [
      bracketMatching(),
      foldGutter(),
      indentOnInput(),
      autocompletion(),
      closeBrackets(),
      history(),
      highlightActiveLine(),
      highlightActiveLineGutter(),
      highlightSelectionMatches(),
      keymap.of([
        ...defaultKeymap,
        ...historyKeymap,
        ...searchKeymap,
        ...foldKeymap,
        ...completionKeymap,
        ...closeBracketsKeymap
      ]),
      EditorView.editable.of(false),
      this.getLineNumbersExtensions(),
      this.getThemeExtensions(),
      this.getLanguageExtensions()
    ];
  }

  private getThemeExtensions(): Extension {
    return this._useDarkTheme ? oneDark : syntaxHighlighting(defaultHighlightStyle);
  }

  private getLanguageExtensions(): Extension {
    return _.isString(this.config.language) && this.config.language !== 'none'
      ? {
          json: json(),
          python: python(),
          shell: StreamLanguage.define(shell),
          xml: xml(),
          yaml: yaml()
        }[this.config.language]
      : [];
  }

  private getLineNumbersExtensions(): Extension {
    return this.config.lineNumbers ? lineNumbers() : [];
  }
}
