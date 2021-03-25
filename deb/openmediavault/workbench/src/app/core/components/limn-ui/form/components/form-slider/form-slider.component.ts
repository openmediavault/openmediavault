import { Component } from '@angular/core';
import { MatSliderChange } from '@angular/material/slider';

import { FormSelectComponent } from '~/app/core/components/limn-ui/form/components/form-select/form-select.component';

@Component({
  selector: 'omv-form-slider',
  templateUrl: './form-slider.component.html',
  styleUrls: ['./form-slider.component.scss']
})
export class FormSliderComponent extends FormSelectComponent {
  onChange(event: MatSliderChange) {
    const control = this.formGroup.get(this.config.name);
    control.setValue(event.value);
  }
}
