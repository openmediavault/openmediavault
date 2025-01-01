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
import { Component, ElementRef, Input, OnInit, ViewChild } from '@angular/core';
import {
  ArcElement,
  Chart,
  ChartDataset,
  ChartType,
  Color,
  DoughnutController,
  Tooltip
} from 'chart.js';
import ChartDataLabels from 'chartjs-plugin-datalabels';
import * as _ from 'lodash';

import {
  ChartDataConfig,
  DashboardWidgetConfig
} from '~/app/core/components/dashboard/models/dashboard-widget-config.model';
import { renderTemplate } from '~/app/functions.helper';
import { translate } from '~/app/i18n.helper';

Chart.register(ArcElement, DoughnutController, Tooltip);

@Component({
  selector: 'omv-dashboard-widget-chart',
  templateUrl: './dashboard-widget-chart.component.html',
  styleUrls: ['./dashboard-widget-chart.component.scss']
})
export class DashboardWidgetChartComponent implements OnInit {
  @ViewChild('chartCtx', { static: true })
  chartCtx: ElementRef;

  @Input()
  config: DashboardWidgetConfig;

  public chart?: Chart;
  // The chart width in percent.
  public chartWidth?: 25 | 50 | 75 | 100;
  public data: Record<string, any>;

  constructor() {}

  ngOnInit(): void {
    this.sanitizeConfig();
    this.chart = this.createChart();
  }

  public labelFormatterByProp(data, prop, context) {
    const value = _.get(data, prop);
    return this.labelFormatter(value, context);
  }

  public labelFormatter(value, context) {
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

  public calcPercent(config: ChartDataConfig, context): number {
    const total = this.getTotal();
    return Math.round(total > 0 ? (_.get(this.data, config.prop) / total) * 100 : 0);
  }

  /**
   * Get the values of all data configurations.
   */
  public getData(): Array<number> {
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
  public getGaugeData(index: number): Array<number> {
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

  public getTotal(): number {
    return _.sum(this.getData());
  }

  public getMaxValue(): number {
    const values = this.getData();
    return Math.max(...values);
  }

  public dataChanged(data: Record<string, any>): void {
    this.data = data;
    this.updateChart();
  }

  protected sanitizeConfig() {
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
        circumference: 360,
        rotation: 0,
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

  protected initDatasets(): Array<ChartDataset> {
    const dataSets: Array<ChartDataset> = [];
    const backgroundColor: Array<Color> = _.map(this.config.chart.dataConfig, 'backgroundColor');
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
        // Set the fill color.
        backgroundColor.push('#0000001a');
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
    const type: ChartType = 'doughnut';
    let cutout: string;
    let displayLegend: boolean;
    let displayDataLabels: boolean;
    let circumference: number = this.config.chart.circumference;
    let rotation: number = this.config.chart.rotation;
    switch (this.config.chart.type) {
      case 'doughnut':
        this.chartWidth = 100;
        cutout = '50%';
        displayLegend = _.defaultTo(this.config.chart.legend.display, true);
        displayDataLabels = _.defaultTo(this.config.chart.label.display, false);
        break;
      case 'advanced-doughnut':
        this.chartWidth = 50;
        cutout = '75%';
        displayLegend = false;
        displayDataLabels = false;
        break;
      case 'gauge':
      case 'advanced-gauge':
        this.chartWidth = this.config.chart.type === 'advanced-gauge' ? 50 : 100;
        cutout = `${80 - this.config.chart.dataConfig.length * 5}%`;
        displayLegend = _.defaultTo(this.config.chart.legend.display, false);
        displayDataLabels = false;
        circumference = 270;
        rotation = -135;
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
        // @ts-ignore
        circumference,
        rotation,
        responsive: true,
        maintainAspectRatio: true,
        cutout,
        plugins: {
          legend: {
            display: displayLegend,
            position: this.config.chart.legend.position
          },
          tooltip: {
            enabled: this.config.chart.tooltips,
            callbacks: {
              label: (tooltipItem): string => {
                const label = translate(tooltipItem.label);
                const formattedValue = this.labelFormatter(tooltipItem.raw, this.chartCtx);
                return `${label}: ${formattedValue}`;
              }
            }
          },
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
}
