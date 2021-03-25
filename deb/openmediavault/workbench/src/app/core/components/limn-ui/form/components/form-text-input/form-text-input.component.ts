import { Component } from '@angular/core';

import { AbstractFormFieldComponent } from '~/app/core/components/limn-ui/form/components/abstract-form-field-component';
import { Icon } from '~/app/shared/enum/icon.enum';
import { ClipboardService } from '~/app/shared/services/clipboard.service';

@Component({
  selector: 'omv-form-text-input',
  templateUrl: './form-text-input.component.html',
  styleUrls: ['./form-text-input.component.scss']
})
export class FormTextInputComponent extends AbstractFormFieldComponent {
  public icon = Icon;

  constructor(private clipboardService: ClipboardService) {
    super();
  }

  onCopyToClipboard() {
    const control = this.formGroup.get(this.config.name);
    this.clipboardService.copy(control.value);
  }
}
