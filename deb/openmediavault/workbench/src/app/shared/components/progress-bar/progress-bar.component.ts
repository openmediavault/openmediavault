import { Component, Input } from '@angular/core';

@Component({
  selector: 'omv-progress-bar',
  templateUrl: './progress-bar.component.html',
  styleUrls: ['./progress-bar.component.scss']
})
export class ProgressBarComponent {
  @Input()
  value = 0.0; // The value in percent [0...100]

  @Input()
  fractionDigits = 1;

  @Input()
  text?: string;

  @Input()
  warningThreshold?: number;
}
