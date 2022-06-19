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
/* eslint-disable @typescript-eslint/member-ordering */
import { BooleanInput, coerceBooleanProperty } from '@angular/cdk/coercion';
import { MediaMatcher } from '@angular/cdk/layout';
import {
  AfterViewInit,
  Component,
  ElementRef,
  HostBinding,
  Input,
  OnDestroy,
  OnInit,
  Optional,
  Self,
  ViewChild
} from '@angular/core';
import { ControlValueAccessor, NgControl } from '@angular/forms';
import { MatFormFieldControl } from '@angular/material/form-field';
import {
  autocompletion,
  closeBrackets,
  closeBracketsKeymap,
  completionKeymap
} from '@codemirror/autocomplete';
import { defaultKeymap, history, historyKeymap } from '@codemirror/commands';
import { json } from '@codemirror/lang-json';
import { xml } from '@codemirror/lang-xml';
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
import { yaml } from '@codemirror/legacy-modes/mode/yaml';
import { highlightSelectionMatches, searchKeymap } from '@codemirror/search';
import { Compartment, EditorState, Extension, StateEffect } from '@codemirror/state';
import { oneDark } from '@codemirror/theme-one-dark';
import {
  EditorView,
  highlightActiveLine,
  highlightActiveLineGutter,
  keymap,
  lineNumbers,
  ViewUpdate
} from '@codemirror/view';
import * as _ from 'lodash';
import { Subject } from 'rxjs';

let nextUniqueId = 0;

@Component({
  // eslint-disable-next-line @angular-eslint/component-selector
  selector: 'mat-form-code-editor',
  exportAs: 'matFormCodeEditor',
  templateUrl: './mat-form-code-editor.component.html',
  styleUrls: ['./mat-form-code-editor.component.scss'],
  providers: [
    {
      provide: MatFormFieldControl,
      useExisting: MatFormCodeEditorComponent
    }
  ]
})
export class MatFormCodeEditorComponent
  implements OnInit, OnDestroy, AfterViewInit, ControlValueAccessor, MatFormFieldControl<string>
{
  // eslint-disable-next-line @typescript-eslint/naming-convention
  static ngAcceptInputType_required: BooleanInput;

  @ViewChild('editorContainer', { static: true })
  _editorContainer: ElementRef;

  /**
   * Implemented as part of MatFormFieldControl.
   */
  public stateChanges = new Subject<void>();
  public controlType = 'code-editor';

  private _value = '';
  private _focused = false;
  private _disabled = false;
  private _required = false;
  private _placeholder: string;
  private _uniqueId = `mat-form-code-editor-${++nextUniqueId}`;
  private _editorState: EditorState;
  private _editorView: EditorView;
  private _mediaQueryList: MediaQueryList;
  private _useDarkTheme = false;

  // @ts-ignore
  private onChange = (_value: any) => {};
  // @ts-ignore
  private onTouched = () => {};

  constructor(private mediaMatcher: MediaMatcher, @Optional() @Self() public ngControl: NgControl) {
    if (!_.isNull(this.ngControl)) {
      this.ngControl.valueAccessor = this;
    }
  }

  @Input()
  lineNumbers?: boolean = true;

  @Input()
  language?: 'json' | 'shell' | 'xml' | 'yaml';

  @Input()
  get value(): string {
    return this._value;
  }
  set value(value: string) {
    if (this._editorView && !_.isEqual(value, this._value)) {
      this._value = value;
      const state: EditorState = this._editorView.state;
      const transaction = state.update({
        changes: { from: 0, to: state.doc.length, insert: value }
      });
      this._editorView.dispatch(transaction);
      this.onChange(this.value);
    }
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  @Input()
  get disabled(): boolean {
    return this._disabled;
  }
  set disabled(value: boolean) {
    this._disabled = coerceBooleanProperty(value);
    if (this._editorView) {
      const stateEffect = new Compartment();
      this._editorView.dispatch({
        effects: stateEffect.reconfigure(EditorView.editable.of(value))
      });
    }
    this.stateChanges.next();
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  @Input()
  get placeholder(): string {
    return this._placeholder;
  }
  set placeholder(value: string) {
    this._placeholder = value;
    this.stateChanges.next();
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  @Input()
  get required(): boolean {
    return this._required;
  }
  set required(required: boolean) {
    this._required = coerceBooleanProperty(required);
    this.stateChanges.next();
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  get focused(): boolean {
    return this._focused;
  }
  set focused(value: boolean) {
    this._focused = value;
    this.stateChanges.next();
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  get errorState() {
    return this.ngControl && !this.ngControl.pristine && !this.ngControl.valid;
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  get empty() {
    return !this.value;
  }

  ngOnInit(): void {
    this._mediaQueryList = this.mediaMatcher.matchMedia('(prefers-color-scheme: dark)');
    this._useDarkTheme = this._mediaQueryList.matches;
    this._mediaQueryList.onchange = (event: MediaQueryListEvent) => {
      this._useDarkTheme = event.matches;
      this._editorView.dispatch({
        effects: StateEffect.reconfigure.of(this.getExtensions())
      });
    };
  }

  ngOnDestroy(): void {
    this._mediaQueryList.onchange = undefined;
  }

  ngAfterViewInit(): void {
    this.createEditor();
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  get id(): string {
    return this._uniqueId;
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  setDescribedByIds(ids: string[]): void {
    // Nothing to do here.
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  onContainerClick(event: MouseEvent): void {
    // Nothing to do here.
  }

  /**
   * Implemented as part of ControlValueAccessor.
   */
  writeValue(value: string) {
    this.value = value;
  }

  /**
   * Implemented as part of ControlValueAccessor.
   */
  registerOnChange(fn: (value: any) => void) {
    this.onChange = fn;
  }

  /**
   * Implemented as part of ControlValueAccessor.
   */
  registerOnTouched(fn: any) {
    this.onTouched = fn;
  }

  /**
   * Implemented as part of ControlValueAccessor.
   */
  setDisabledState(disabled: boolean): void {
    this.disabled = disabled;
  }

  @HostBinding('class.floating')
  get shouldLabelFloat() {
    return this.focused || !this.empty;
  }

  private createEditor(): void {
    this._editorState = EditorState.create({
      doc: this.value,
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
      EditorView.editable.of(!this.disabled),
      EditorView.updateListener.of((vu: ViewUpdate) => {
        if (vu.docChanged) {
          this._value = vu.state.doc.toString();
          this.onChange(this.value);
        }
        if (vu.focusChanged) {
          this.focused = vu.view.hasFocus;
        }
      }),
      this.getLineNumbersExtensions(),
      this.getThemeExtensions(),
      this.getLanguageExtensions()
    ];
  }

  private getThemeExtensions(): Extension {
    return this._useDarkTheme ? oneDark : syntaxHighlighting(defaultHighlightStyle);
  }

  private getLanguageExtensions(): Extension {
    return _.isString(this.language)
      ? {
          xml: xml(),
          json: json(),
          shell: StreamLanguage.define(shell),
          yaml: StreamLanguage.define(yaml)
        }[this.language]
      : [];
  }

  private getLineNumbersExtensions(): Extension {
    return this.lineNumbers ? lineNumbers() : [];
  }
}
