import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { ArrayPipe } from '~/app/shared/pipes/array.pipe';
import { BinaryUnitPipe } from '~/app/shared/pipes/binary-unit.pipe';
import { DefaultToPipe } from '~/app/shared/pipes/default-to.pipe';
import { HttpErrorResponsePipe } from '~/app/shared/pipes/http-error-response.pipe';
import { JoinPipe } from '~/app/shared/pipes/join.pipe';
import { LocaleDatePipe } from '~/app/shared/pipes/locale-date.pipe';
import { MapPipe } from '~/app/shared/pipes/map.pipe';
import { MaxPipe } from '~/app/shared/pipes/max.pipe';
import { NotAvailablePipe } from '~/app/shared/pipes/not-available.pipe';
import { TemplatePipe } from '~/app/shared/pipes/template.pipe';
import { ToBooleanPipe } from '~/app/shared/pipes/to-boolean.pipe';
import { TruncatePipe } from '~/app/shared/pipes/truncate.pipe';
import { TrustHtmlPipe } from '~/app/shared/pipes/trust-html.pipe';
import { UpperFirstPipe } from '~/app/shared/pipes/upper-first.pipe';

@NgModule({
  declarations: [
    ToBooleanPipe,
    LocaleDatePipe,
    TruncatePipe,
    NotAvailablePipe,
    DefaultToPipe,
    JoinPipe,
    BinaryUnitPipe,
    HttpErrorResponsePipe,
    MapPipe,
    ArrayPipe,
    TemplatePipe,
    TrustHtmlPipe,
    UpperFirstPipe,
    MaxPipe
  ],
  imports: [CommonModule],
  exports: [
    ToBooleanPipe,
    LocaleDatePipe,
    TruncatePipe,
    NotAvailablePipe,
    DefaultToPipe,
    JoinPipe,
    BinaryUnitPipe,
    HttpErrorResponsePipe,
    MapPipe,
    ArrayPipe,
    TemplatePipe,
    TrustHtmlPipe,
    UpperFirstPipe,
    MaxPipe
  ],
  providers: [
    ToBooleanPipe,
    LocaleDatePipe,
    TruncatePipe,
    NotAvailablePipe,
    DefaultToPipe,
    JoinPipe,
    BinaryUnitPipe,
    HttpErrorResponsePipe,
    MapPipe,
    ArrayPipe,
    TemplatePipe,
    TrustHtmlPipe,
    UpperFirstPipe
  ]
})
export class PipesModule {}
