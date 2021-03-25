import { Component, ElementRef, OnInit, ViewChild } from '@angular/core';
import { Chart, ChartColor, ChartDataSets } from 'chart.js';
import ChartDataLabels from 'chartjs-plugin-datalabels';
import * as _ from 'lodash';
import { EMPTY, Observable } from 'rxjs';
import { map } from 'rxjs/operators';

import { ChartDataConfig } from '~/app/core/components/dashboard/models/dashboard-widget-config.model';
import { AbstractDashboardWidgetComponent } from '~/app/core/components/dashboard/widgets/abstract-dashboard-widget-component';
import { formatDeep, renderTemplate } from '~/app/functions.helper';
import { translate } from '~/app/i18n.helper';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  selector: 'omv-dashboard-widget-chart',
  templateUrl: './widget-chart.component.html',
  styleUrls: ['./widget-chart.component.scss']
})
export class WidgetChartComponent
  extends AbstractDashboardWidgetComponent<Record<string, any>>
  implements OnInit {
  @ViewChild('chartCtx', { static: true })
  chartCtx: ElementRef;

  chart?: Chart;
  chartWidth?: number;

  constructor(private rpcService: RpcService) {
    super();
    this.subscriptions.add(this.loadDataEvent.subscribe(() => this.updateChart()));
  }

  ngOnInit(): void {
    super.ngOnInit();
    this.chart = this.createChart();
  }

  labelFormatterByProp(data, prop, context) {
    const value = _.get(data, prop);
    return this.labelFormatter(value, context);
  }

  labelFormatter(value, context) {
    let result;
    switch (this.config.chart.label.formatter) {
      case 'template':
        result = renderTemplate(this.config.chart.label.formatterConfig, { value });
        break;
      case 'label':
        result = context.chart.data.labels[context.dataIndex];
        break;
      default:
        result = value;
        break;
    }
    return result;
  }

  calcPercent(config: ChartDataConfig, context): number {
    const total = this.getTotal();
    return Math.round(total > 0 ? (_.get(this.data, config.prop) / total) * 100 : 0);
  }

  /**
   * Get the values of all data configurations.
   */
  getData(): Array<number> {
    const data: Array<number> = [];
    _.forEach(this.config.chart.dataConfig, (dataConfig) => {
      const value = parseFloat(_.get(this.data, dataConfig.prop));
      data.push(value);
    });
    return data;
  }

  /**
   * Get the values for the specified chart dataset. The values will be
   * adapted automatically to the configured min/max boundaries.
   */
  getGaugeData(index: number): Array<number> {
    const values: Array<number> = this.getData();
    const data: Array<number> = _.fill(Array(values.length), 0);
    let minValue = Math.min(...values);
    let maxValue = Math.max(...values);
    if (!_.isUndefined(this.config.chart.min)) {
      minValue = Math.min(this.config.chart.min, minValue);
    }
    if (!_.isUndefined(this.config.chart.max)) {
      maxValue = Math.max(this.config.chart.max, maxValue);
    }
    // Ensure the maximum value is NOT zero, otherwise the chart will
    // not show anything.
    if (maxValue === 0) {
      maxValue = 100;
    }
    // Adapt the current value to the specified boundaries.
    const value = Math.min(Math.max(minValue, values[index]), maxValue);
    data[index] = value;
    // Add difference as additional value to draw the dataset line.
    data.push(maxValue - value);
    return data;
  }

  getTotal(): number {
    return _.sum(this.getData());
  }

  getMaxValue(): number {
    const values = this.getData();
    return Math.max(...values);
  }

  protected sanitizeConfig() {
    super.sanitizeConfig();
    _.defaultsDeep(this.config, {
      reloadPeriod: 10000,
      chart: {
        type: 'doughnut',
        label: {
          display: false,
          formatter: 'none',
          color: 'black',
          align: 'center',
          anchor: 'center'
        },
        circumference: 2 * Math.PI,
        rotation: -0.5 * Math.PI,
        tooltips: false,
        legend: {
          position: 'bottom'
        },
        displayValue: false
      }
    });
    switch (this.config.chart.type) {
      case 'advanced-doughnut':
        _.merge(this.config.chart, {
          tooltips: false
        });
        break;
      case 'advanced-gauge':
        _.merge(this.config.chart, {
          displayValue: false,
          legend: {
            display: false
          },
          tooltips: false
        });
        break;
    }
  }

  protected initDatasets(): Array<ChartDataSets> {
    const dataSets: Array<ChartDataSets> = [];
    const backgroundColor: ChartColor = _.map(this.config.chart.dataConfig, 'backgroundColor');
    switch (this.config.chart.type) {
      case 'doughnut':
      case 'advanced-doughnut':
        // These chart types have only one dataset.
        dataSets.push({
          data: [],
          backgroundColor
        });
        break;
      case 'gauge':
      case 'advanced-gauge':
        // This type of chart can have multiple datasets.
        _.forEach(this.config.chart.dataConfig, (dataConfig) => {
          dataSets.push({
            data: [],
            backgroundColor,
            label: dataConfig.label
          });
        });
        break;
    }
    return dataSets;
  }

  protected createChart(): Chart {
    let type: string;
    let cutoutPercentage: number;
    let displayLegend: boolean;
    let displayDataLabels: boolean;
    let circumference: number | undefined = this.config.chart.circumference;
    let rotation: number | undefined = this.config.chart.rotation;
    switch (this.config.chart.type) {
      case 'doughnut':
        this.chartWidth = 100;
        type = 'doughnut';
        cutoutPercentage = 50;
        displayLegend = _.defaultTo(this.config.chart.legend.display, true);
        displayDataLabels = _.defaultTo(this.config.chart.label.display, false);
        break;
      case 'advanced-doughnut':
        this.chartWidth = 50;
        type = 'doughnut';
        cutoutPercentage = 75;
        displayLegend = false;
        displayDataLabels = false;
        break;
      case 'gauge':
      case 'advanced-gauge':
        this.chartWidth = this.config.chart.type === 'advanced-gauge' ? 50 : 100;
        type = 'doughnut';
        cutoutPercentage = 80 - this.config.chart.dataConfig.length * 5;
        displayLegend = _.defaultTo(this.config.chart.legend.display, false);
        displayDataLabels = false;
        circumference = 1.5 * Math.PI;
        rotation = -1.25 * Math.PI;
        break;
    }
    return new Chart(this.chartCtx.nativeElement, {
      type,
      data: {
        labels: _.map(this.config.chart.dataConfig, 'label'),
        datasets: this.initDatasets()
      },
      plugins: [ChartDataLabels],
      options: {
        circumference,
        rotation,
        responsive: true,
        maintainAspectRatio: false,
        cutoutPercentage,
        legend: {
          display: displayLegend,
          position: this.config.chart.legend.position
        },
        tooltips: {
          enabled: this.config.chart.tooltips,
          callbacks: {
            label: (tooltipItem, data): string => {
              const index = tooltipItem.index;
              const dataSet = data.datasets[tooltipItem.datasetIndex];
              const value = dataSet.data[index];
              const label = translate(data.labels[index] as string);
              const formattedValue = this.labelFormatter(value, this.chartCtx);
              return `${label}: ${formattedValue}`;
            }
          }
        },
        plugins: {
          datalabels: {
            display: displayDataLabels,
            color: this.config.chart.label.color,
            formatter: this.labelFormatter.bind(this),
            align: this.config.chart.label.align,
            anchor: this.config.chart.label.anchor
          }
        }
      }
    });
  }

  protected updateChart() {
    if (this.chart) {
      let data: Array<number> = [];
      switch (this.config.chart.type) {
        case 'doughnut':
        case 'advanced-doughnut':
          // These chart types have only one dataset.
          data = this.getData();
          _.set(this.chart, 'data.datasets.0.data', data);
          break;
        case 'gauge':
        case 'advanced-gauge':
          // This type of chart can have multiple datasets.
          _.forEach(this.config.chart.dataConfig, (dataConfig, index) => {
            data = this.getGaugeData(index);
            _.set(this.chart, `data.datasets.${index}.data`, data);
          });
          break;
      }
      this.chart.update();
    }
  }

  protected loadData(): Observable<Record<string, any>> {
    if (!_.isPlainObject(this.config.chart.request)) {
      return EMPTY;
    }
    return this.rpcService
      .request(
        this.config.chart.request.service,
        this.config.chart.request.method,
        this.config.chart.request.params
      )
      .pipe(
        map((res) => {
          if (_.isPlainObject(this.config.chart.request.transform)) {
            const tmp = formatDeep(this.config.chart.request.transform, res);
            _.merge(res, tmp);
          }
          return res;
        })
      );
  }
}
