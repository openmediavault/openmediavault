/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2026 Volker Theile
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
import { Component, ElementRef, Input, OnDestroy, OnInit, ViewChild } from '@angular/core';
import {
  ArcElement,
  Chart,
  ChartDataset,
  ChartType,
  Color,
  DoughnutController,
  Plugin,
  Tooltip
} from 'chart.js';
import { color as parseColor } from 'chart.js/helpers';
import ChartDataLabels from 'chartjs-plugin-datalabels';
import * as _ from 'lodash';
import { Subscription } from 'rxjs';

import {
  ChartDataConfig,
  DashboardWidgetConfig
} from '~/app/core/components/dashboard/models/dashboard-widget-config.model';
import { renderTemplate } from '~/app/functions.helper';
import { translate } from '~/app/i18n.helper';
import { PrefersColorSchemeService } from '~/app/shared/services/prefers-color-scheme.service';

Chart.register(ArcElement, DoughnutController, Tooltip);

/**
 * Draw the full track underneath the values, so only the track is a
 * complete ring. Redraw the value arcs on top of the chart with a
 * gradient along the arc. Multi-ring gauges get a gap between their
 * rings.
 */
export const chartStylePlugin: Plugin<
  'doughnut',
  { gauge: boolean; getTrackColor?: () => string }
> = {
  id: 'omvChartStyle',
  beforeDatasetsDraw: (chart, args, options) => {
    const ctx = chart.ctx;
    _.forEach(chart.data.datasets, (dataset, index) => {
      const elements: Array<Record<string, any>> = chart.getDatasetMeta(index).data;
      if (_.isEmpty(elements)) {
        return;
      }
      const first = _.first(elements);
      const width = first.outerRadius - first.innerRadius;
      const radius = (first.outerRadius + first.innerRadius) / 2;
      // Use the configured angles, not those of the elements, so the
      // track stays still while the values are animated. Chart.js starts
      // at the top of the circle.
      const config: Record<string, any> = chart.config.options;
      const start = ((config.rotation - 90) * Math.PI) / 180;
      const circumference = (config.circumference * Math.PI) / 180;
      // Inset both ends of a gauge by half the width, the round caps
      // fill them again. A doughnut track is a full circle.
      const inset = options.gauge ? width / 2 / radius : 0;
      const end = start + (options.gauge ? circumference : 2 * Math.PI);
      ctx.save();
      ctx.strokeStyle = options.getTrackColor();
      ctx.lineWidth = width;
      ctx.lineCap = 'round';
      ctx.beginPath();
      ctx.arc(first.x, first.y, radius, start + inset, end - inset);
      ctx.stroke();
      ctx.restore();
    });
  },
  afterDatasetsDraw: (chart, args, options) => {
    const ctx = chart.ctx;
    if (_.isFunction(ctx.createConicGradient)) {
      // A gauge draws the value of each dataset at its own index,
      // followed by the unfilled part of the arc. A doughnut has one
      // dataset where every element is a value.
      const arcs: Array<[Record<string, any>, any]> = options.gauge
        ? _.map(chart.data.datasets, (dataset, index) => [
            chart.getDatasetMeta(index).data[index],
            dataset.backgroundColor[index]
          ])
        : _.map(chart.getDatasetMeta(0).data, (element, index) => [
            element,
            chart.data.datasets[0].backgroundColor[index]
          ]);
      _.forEach(arcs, ([element, arcColor]) => {
        if (!element || !_.isString(arcColor)) {
          return;
        }
        // Short arcs, e.g. the dot of a tiny value, keep a solid color.
        const width = element.outerRadius - element.innerRadius;
        const radius = (element.outerRadius + element.innerRadius) / 2;
        if ((element.endAngle - element.startAngle) * radius < 2 * width) {
          return;
        }
        // Fade the arc in from a lighter shade of its color.
        const fill = element.options.backgroundColor;
        const gradient = ctx.createConicGradient(element.startAngle, element.x, element.y);
        const stop = Math.min((element.endAngle - element.startAngle) / (2 * Math.PI), 1);
        gradient.addColorStop(
          0,
          parseColor(arcColor as string)
            .mix(parseColor('#ffffff'), 0.65)
            .rgbString()
        );
        gradient.addColorStop(stop, arcColor);
        element.options.backgroundColor = gradient;
        element.draw(ctx);
        element.options.backgroundColor = fill;
      });
    }
    // Cut a transparent gap between the rings of a multi-ring gauge.
    if (options.gauge && chart.data.datasets.length > 1) {
      ctx.save();
      ctx.globalCompositeOperation = 'destination-out';
      ctx.lineWidth = 2;
      _.forEach(_.range(1, chart.data.datasets.length), (index) => {
        const element: Record<string, any> = chart.getDatasetMeta(index).data[0];
        if (element) {
          ctx.beginPath();
          ctx.arc(element.x, element.y, element.outerRadius, 0, 2 * Math.PI);
          ctx.stroke();
        }
      });
      ctx.restore();
    }
  }
};

@Component({
  selector: 'omv-dashboard-widget-chart',
  templateUrl: './dashboard-widget-chart.component.html',
  styleUrls: ['./dashboard-widget-chart.component.scss']
})
export class DashboardWidgetChartComponent implements OnInit, OnDestroy {
  @ViewChild('chartCtx', { static: true })
  chartCtx: ElementRef;

  @Input()
  config: DashboardWidgetConfig;

  public chart?: Chart;
  // The chart width in percent.
  public chartWidth?: 25 | 50 | 75 | 100;
  public data: Record<string, any>;

  private subscriptions: Subscription = new Subscription();
  // Whether the chart has been drawn with data yet.
  private drawn = false;

  constructor(private prefersColorSchemeService: PrefersColorSchemeService) {}

  ngOnInit(): void {
    this.sanitizeConfig();
    this.chart = this.createChart();
    this.subscriptions.add(
      this.prefersColorSchemeService.change$.subscribe(() => this.applyColorScheme())
    );
  }

  ngOnDestroy(): void {
    this.subscriptions.unsubscribe();
  }

  /**
   * Get the color of the track that is drawn underneath the values.
   */
  public getTrackColor(): string {
    const trackColor =
      this.prefersColorSchemeService.current === 'dark' ? '#ffffff1f' : '#0000001a';
    return _.includes(['gauge', 'advanced-gauge'], this.config.chart.type)
      ? _.get(this.config.chart, 'gauge.trackColor', trackColor)
      : trackColor;
  }

  /**
   * Get the smallest part of the chart a value is drawn with, so that a
   * tiny value is still visible as a dot as wide as the ring.
   */
  public getMinArcFraction(): number {
    const options: Record<string, any> = this.chart.config.options;
    const cutout = parseFloat(options.cutout) / 100;
    // The angle of an arc in the middle of the ring that is as long as
    // the ring is wide.
    const angle = (2 * (1 - cutout)) / (1 + cutout);
    return angle / ((options.circumference * Math.PI) / 180);
  }

  /**
   * Enlarge tiny non-zero values to the given fraction of the total so
   * that they stay visible. The other values shrink proportionally.
   * This only changes how the values are drawn.
   */
  public enlargeTinyValues(values: Array<number>, fraction: number): Array<number> {
    const total = _.sum(values);
    if (!(total > 0)) {
      return values;
    }
    const tiny = _.map(values, (value) => value > 0 && value / total < fraction);
    const count = _.filter(tiny).length;
    if (count === 0 || count * fraction >= 1) {
      return values;
    }
    const rest = _.sum(_.filter(values, (value, index) => !tiny[index]));
    const scale = (total * (1 - count * fraction)) / rest;
    return _.map(values, (value, index) => (tiny[index] ? total * fraction : value * scale));
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

  /**
   * Get the arc color for the specified value. This is the color of the
   * highest threshold that is lower than or equal to the value, otherwise
   * the background color of the data configuration.
   */
  public getThresholdColor(config: ChartDataConfig, value: number): Color {
    const threshold = _.findLast(
      _.sortBy(this.config.chart.gauge?.thresholds, 'value'),
      (item) => value >= item.value
    );
    return _.defaultTo(threshold?.color, config.backgroundColor);
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
    // Rounded arcs without a border. The radius is limited by Chart.js
    // to half of the arc width, so the arc ends are fully rounded.
    const arcOptions = {
      borderRadius: 20,
      borderWidth: 0
    };
    switch (this.config.chart.type) {
      case 'doughnut':
      case 'advanced-doughnut':
        // These chart types have only one dataset.
        dataSets.push({
          data: [],
          backgroundColor,
          ...arcOptions,
          spacing: 2
        });
        break;
      case 'gauge':
      case 'advanced-gauge':
        // The unfilled part of the arc is transparent, the full track
        // is drawn underneath by the chart style plugin.
        backgroundColor.push('transparent');
        // This type of chart can have multiple datasets.
        _.forEach(this.config.chart.dataConfig, (dataConfig) => {
          dataSets.push({
            data: [],
            // Each dataset needs its own colors, the arc color may
            // change depending on the value.
            backgroundColor: [...backgroundColor],
            label: dataConfig.label,
            ...arcOptions
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
    let gauge = false;
    switch (this.config.chart.type) {
      case 'doughnut':
        this.chartWidth = 100;
        cutout = '50%';
        displayLegend = _.defaultTo(this.config.chart.legend.display, true);
        displayDataLabels = _.defaultTo(this.config.chart.label.display, false);
        break;
      case 'advanced-doughnut':
        this.chartWidth = 50;
        cutout = '82%';
        displayLegend = false;
        displayDataLabels = false;
        break;
      case 'gauge':
      case 'advanced-gauge':
        this.chartWidth = this.config.chart.type === 'advanced-gauge' ? 50 : 100;
        cutout = _.get(
          this.config.chart,
          'gauge.cutout',
          `${90 - this.config.chart.dataConfig.length * 5}%`
        );
        displayLegend = _.defaultTo(this.config.chart.legend.display, false);
        displayDataLabels = false;
        circumference = _.get(this.config.chart, 'gauge.circumference', 270);
        rotation = _.get(this.config.chart, 'gauge.rotation', -135);
        gauge = true;
        break;
    }
    return new Chart(this.chartCtx.nativeElement, {
      type,
      data: {
        labels: _.map(this.config.chart.dataConfig, 'label'),
        datasets: this.initDatasets()
      },
      plugins: [ChartDataLabels, chartStylePlugin],
      options: {
        // @ts-ignore
        circumference,
        rotation,
        responsive: true,
        maintainAspectRatio: true,
        cutout,
        // Value changes glide gently, no animation if the user prefers
        // reduced motion. See updateChart() for the first values.
        animation: window.matchMedia?.('(prefers-reduced-motion: reduce)').matches
          ? false
          : { duration: 800, easing: 'easeOutCubic' },
        plugins: {
          // @ts-ignore
          omvChartStyle: {
            gauge,
            // Called on every draw to follow the color scheme. Chart.js
            // does not resolve functions in plugin options.
            getTrackColor: (): string => this.getTrackColor()
          },
          legend: {
            display: displayLegend,
            position: this.config.chart.legend.position
          },
          tooltip: {
            enabled: this.config.chart.tooltips,
            callbacks: {
              label: (tooltipItem): string => {
                const label = translate(tooltipItem.label);
                // Show the real value, tiny values are drawn enlarged.
                const value = _.get(this.getData(), tooltipItem.dataIndex, tooltipItem.raw);
                const formattedValue = this.labelFormatter(value, this.chartCtx);
                return `${label}: ${formattedValue}`;
              }
            }
          },
          datalabels: {
            display: displayDataLabels,
            color: this.config.chart.label.color,
            // Show the real value, tiny values are drawn enlarged.
            formatter: (value, context) =>
              this.labelFormatter(_.get(this.getData(), context.dataIndex, value), context),
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
          data = this.enlargeTinyValues(this.getData(), this.getMinArcFraction());
          _.set(this.chart, 'data.datasets.0.data', data);
          break;
        case 'gauge':
        case 'advanced-gauge':
          // This type of chart can have multiple datasets.
          _.forEach(this.config.chart.dataConfig, (dataConfig, index) => {
            data = this.getGaugeData(index);
            // Keep a tiny value visible, the last value is the unfilled
            // part of the arc.
            [data[index], data[data.length - 1]] = this.enlargeTinyValues(
              [data[index], _.last(data)],
              this.getMinArcFraction()
            );
            _.set(this.chart, `data.datasets.${index}.data`, data);
            if (!_.isEmpty(this.config.chart.gauge?.thresholds)) {
              _.set(
                this.chart,
                `data.datasets.${index}.backgroundColor.${index}`,
                this.getThresholdColor(dataConfig, this.getData()[index])
              );
            }
          });
          break;
      }
      // A gauge sweeps from zero to its first value. The segments of a
      // doughnut appear at once, letting them all grow in at the same
      // time makes the dashboard busy while it loads.
      const gauge = _.includes(['gauge', 'advanced-gauge'], this.config.chart.type);
      this.chart.update(this.drawn || gauge ? undefined : 'none');
      this.drawn = true;
    }
  }

  /**
   * Adapt the unfilled part of gauge arcs to the current color scheme.
   */
  protected applyColorScheme() {
    // Redraw without animation, the track color is read on every draw.
    this.chart?.update('none');
  }
}
