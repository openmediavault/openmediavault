import { Component } from '@angular/core';

import { AbstractFormFieldComponent } from '~/app/core/components/limn-ui/form/components/abstract-form-field-component';
import { Icon } from '~/app/shared/enum/icon.enum';
import { ClipboardService } from '~/app/shared/services/clipboard.service';

@Component({
  selector: 'omv-form-password-input',
  templateUrl: './form-password-input.component.html',
  styleUrls: ['./form-password-input.component.scss']
})
export class FormPasswordInputComponent extends AbstractFormFieldComponent {
  public type = 'password';
  public icon = Icon;

  constructor(private clipboardService: ClipboardService) {
    super();
  }

  onCopyToClipboard() {
    const control = this.formGroup.get(this.config.name);
    this.clipboardService.copy(control.value);
  }

  onToggleVisibility() {
    this.type = this.type === 'password' ? 'text' : 'password';
  }

  protected sanitizeConfig() {
    super.sanitizeConfig();
    // Hide the 'Copy to clipboard' button if the browser does not
    // support that feature.
    if (!this.clipboardService.canCopy()) {
      this.config.hasCopyToClipboardButton = false;
    }
  }
}
