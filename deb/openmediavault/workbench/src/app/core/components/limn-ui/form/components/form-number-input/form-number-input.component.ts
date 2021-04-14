import { Component } from '@angular/core';

import { AbstractFormFieldComponent } from '~/app/core/components/limn-ui/form/components/abstract-form-field-component';
import { Icon } from '~/app/shared/enum/icon.enum';
import { ClipboardService } from '~/app/shared/services/clipboard.service';

@Component({
  selector: 'omv-form-number-input',
  templateUrl: './form-number-input.component.html',
  styleUrls: ['./form-number-input.component.scss']
})
export class FormNumberInputComponent extends AbstractFormFieldComponent {
  public icon = Icon;

  constructor(private clipboardService: ClipboardService) {
    super();
  }

  onCopyToClipboard() {
    const control = this.formGroup.get(this.config.name);
    this.clipboardService.copy(control.value);
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
