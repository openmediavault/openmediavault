import { Component, OnInit } from '@angular/core';
import { Subscription } from 'rxjs';

import { AbstractFormFieldComponent } from '~/app/core/components/intuition/form/components/abstract-form-field-component';
import { Unsubscribe } from '~/app/decorators';
import { format } from '~/app/functions.helper';

@Component({
  selector: 'omv-form-card',
  templateUrl: './form-card.component.html',
  styleUrls: ['./form-card.component.scss']
})
export class FormCardComponent extends AbstractFormFieldComponent implements OnInit {
  @Unsubscribe()
  private subscriptions = new Subscription();

  protected content: string;

  override ngOnInit(): void {
    super.ngOnInit();

    this.subscriptions.add(
      this.formGroup.valueChanges.subscribe((values: Record<string, any>) => {
        this.content = format(this.config.text, { ...this.pageContext, ...values });
      })
    );
  }
}
