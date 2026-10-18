/* eslint-disable max-len */
import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import {
  chartStylePlugin,
  DashboardWidgetChartComponent
} from '~/app/core/components/dashboard/dashboard-widget-chart/dashboard-widget-chart.component';
import { PrefersColorSchemeService } from '~/app/shared/services/prefers-color-scheme.service';
import { TestingModule } from '~/app/testing.module';

describe('DashboardWidgetChartComponent', () => {
  let component: DashboardWidgetChartComponent;
  let fixture: ComponentFixture<DashboardWidgetChartComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [DashboardWidgetChartComponent],
      imports: [DashboardModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(DashboardWidgetChartComponent);
    component = fixture.componentInstance;
    component.config = {
      id: 'c2eecbb8-f40f-4110-99f1-4cb037edce68',
      type: 'chart',
      title: 'foo',
      chart: {
        type: 'doughnut',
        dataConfig: []
      }
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  const createChart = (chart: Record<string, any>) => {
    fixture = TestBed.createComponent(DashboardWidgetChartComponent);
    component = fixture.componentInstance;
    component.config = {
      id: 'c2eecbb8-f40f-4110-99f1-4cb037edce68',
      type: 'chart',
      title: 'foo',
      chart: {
        dataConfig: [],
        ...chart
      } as any
    };
    fixture.detectChanges();
  };

  describe('doughnut', () => {
    const memory = {
      type: 'advanced-doughnut',
      dataConfig: [
        { label: 'Used', prop: 'used', backgroundColor: '#f44336' },
        { label: 'Free', prop: 'free', backgroundColor: '#4cd964' }
      ]
    };

    it('should use the modern style', () => {
      createChart(memory);
      const options: Record<string, any> = component.chart.config.options;
      const dataset: Record<string, any> = component.chart.config.data.datasets[0];
      expect(options.circumference).toBe(360);
      expect(options.rotation).toBe(0);
      expect(options.cutout).toBe('82%');
      expect(options.plugins.omvChartStyle.gauge).toBe(false);
      expect(component.chart.config.plugins).toHaveLength(2);
      expect(dataset.borderRadius).toBe(20);
      expect(dataset.borderWidth).toBe(0);
      expect(dataset.spacing).toBe(2);
      expect(dataset.backgroundColor).toEqual(['#f44336', '#4cd964']);
    });

    it('should animate gently', () => {
      createChart(memory);
      expect((component.chart.config.options as any).animation).toEqual({
        duration: 800,
        easing: 'easeOutCubic'
      });
    });

    it('should draw the first values of a doughnut without animation', () => {
      createChart(memory);
      const update = jest.spyOn(component.chart, 'update');
      component.dataChanged({ used: 1, free: 30 });
      expect(update).toHaveBeenLastCalledWith('none');
      // Later changes glide to their new values.
      component.dataChanged({ used: 2, free: 29 });
      expect(update).toHaveBeenLastCalledWith(undefined);
    });

    it('should not animate a color scheme change', () => {
      createChart(memory);
      component.dataChanged({ used: 1, free: 30 });
      const update = jest.spyOn(component.chart, 'update');
      TestBed.inject(PrefersColorSchemeService).set('dark');
      expect(update).toHaveBeenLastCalledWith('none');
      TestBed.inject(PrefersColorSchemeService).set('light');
    });

    it('should not animate if reduced motion is preferred', () => {
      const matchMedia = window.matchMedia;
      window.matchMedia = jest.fn((query: string) => ({
        matches: query === '(prefers-reduced-motion: reduce)'
      })) as any;
      try {
        createChart(memory);
        expect((component.chart.config.options as any).animation).toBe(false);
      } finally {
        window.matchMedia = matchMedia;
      }
    });

    it('should keep the data unchanged', () => {
      createChart(memory);
      component.dataChanged({ used: 1, free: 30.18 });
      expect(component.chart.config.data.datasets[0].data).toEqual([1, 30.18]);
      expect(component.getTotal()).toBe(31.18);
      expect(component.calcPercent(memory.dataConfig[0], null)).toBe(3);
    });

    it('should keep room for data labels in plain doughnuts', () => {
      createChart({ ...memory, type: 'doughnut' });
      expect((component.chart.config.options as any).cutout).toBe('50%');
    });

    it('should draw tiny values enlarged but show their real value', () => {
      // The misses of a ZFS ARC hit ratio, 0.05% of the total.
      createChart({
        type: 'advanced-doughnut',
        dataConfig: [
          { label: 'Misses', prop: 'misses', backgroundColor: '#f44336' },
          { label: 'Hits', prop: 'hits', backgroundColor: '#4cd964' }
        ]
      });
      component.dataChanged({ misses: 281, hits: 573636 });
      const fraction = component.getMinArcFraction();
      // A ring cut out by 82% is 18% wide, so the dot is about 3% of it.
      expect(fraction).toBeCloseTo((2 * 0.18) / 1.82 / (2 * Math.PI));
      const data: Array<number> = component.chart.config.data.datasets[0].data as any;
      expect(data[0] / (data[0] + data[1])).toBeCloseTo(fraction);
      expect(data[0] + data[1]).toBeCloseTo(573917);
      // Labels, tooltips and percentages use the real values.
      const options: Record<string, any> = component.chart.config.options;
      expect(options.plugins.datalabels.formatter(data[0], { dataIndex: 0 })).toBe(281);
      expect(component.calcPercent(component.config.chart.dataConfig[0], null)).toBe(0);
    });

    it('should not draw zero values', () => {
      createChart(memory);
      component.dataChanged({ used: 0, free: 30 });
      expect(component.chart.config.data.datasets[0].data).toEqual([0, 30]);
    });

    it('should use the default track color', () => {
      createChart({ ...memory, gauge: { trackColor: '#000000' } });
      expect(component.getTrackColor()).toBe('#0000001a');
    });

    it('should ignore gauge settings', () => {
      createChart({ ...memory, gauge: { circumference: 180, trackColor: '#000000' } });
      const options: Record<string, any> = component.chart.config.options;
      expect(options.circumference).toBe(360);
      expect(component.chart.config.data.datasets[0].backgroundColor).toEqual([
        '#f44336',
        '#4cd964'
      ]);
    });
  });

  describe('gauge', () => {
    const createGauge = (chart: Record<string, any> = {}) =>
      createChart({
        type: 'gauge',
        min: 0,
        max: 100,
        dataConfig: [{ label: 'CPU', prop: 'cpu', backgroundColor: '#4cd964' }],
        ...chart
      });

    it('should use the modern style', () => {
      createGauge();
      const options: Record<string, any> = component.chart.config.options;
      const dataset: Record<string, any> = component.chart.config.data.datasets[0];
      expect(options.circumference).toBe(270);
      expect(options.rotation).toBe(-135);
      expect(options.cutout).toBe('85%');
      expect(options.plugins.omvChartStyle.gauge).toBe(true);
      expect(options.plugins.omvChartStyle.getTrackColor()).toBe('#0000001a');
      expect(dataset.borderRadius).toBe(20);
      expect(dataset.borderWidth).toBe(0);
      // The unfilled part is transparent, the track is drawn underneath.
      expect(dataset.backgroundColor).toEqual(['#4cd964', 'transparent']);
    });

    it('should keep the data unchanged', () => {
      createGauge();
      component.dataChanged({ cpu: 90 });
      const dataset: Record<string, any> = component.chart.config.data.datasets[0];
      expect(dataset.data).toEqual([90, 10]);
      expect(dataset.backgroundColor).toEqual(['#4cd964', 'transparent']);
    });

    it('should sweep from zero to the first value', () => {
      createGauge();
      const update = jest.spyOn(component.chart, 'update');
      component.dataChanged({ cpu: 40 });
      expect(update).toHaveBeenLastCalledWith(undefined);
    });

    it('should draw a tiny value enlarged', () => {
      createGauge();
      component.dataChanged({ cpu: 0.1 });
      const data: Array<number> = component.chart.config.data.datasets[0].data as any;
      const fraction = component.getMinArcFraction();
      expect(data[0]).toBeCloseTo(fraction * 100);
      expect(data[0] + data[1]).toBeCloseTo(100);
      // The displayed value is the real one.
      expect(component.getMaxValue()).toBe(0.1);
    });

    it('should make rings thinner the more datasets there are', () => {
      createGauge({
        type: 'advanced-gauge',
        dataConfig: [
          { label: '1 min', prop: 'a', backgroundColor: '#4cd964' },
          { label: '5 min', prop: 'b', backgroundColor: '#2196f3' },
          { label: '15 min', prop: 'c', backgroundColor: '#ff9500' }
        ]
      });
      expect((component.chart.config.options as any).cutout).toBe('75%');
      expect(component.chart.config.data.datasets).toHaveLength(3);
    });

    it('should apply gauge settings', () => {
      createGauge({
        gauge: {
          circumference: 180,
          rotation: -90,
          cutout: '80%',
          trackColor: '#00000033'
        }
      });
      const options: Record<string, any> = component.chart.config.options;
      expect(options.circumference).toBe(180);
      expect(options.rotation).toBe(-90);
      expect(options.cutout).toBe('80%');
      expect((options.plugins as any).omvChartStyle.getTrackColor()).toBe('#00000033');
    });

    it('should adapt to the dark color scheme', () => {
      createGauge();
      const prefersColorSchemeService = TestBed.inject(PrefersColorSchemeService);
      prefersColorSchemeService.set('dark');
      const style: Record<string, any> = (component.chart.config.options.plugins as any)
        .omvChartStyle;
      expect(style.getTrackColor()).toBe('#ffffff1f');
      prefersColorSchemeService.set('light');
      expect(style.getTrackColor()).toBe('#0000001a');
    });

    it('should color the arc by thresholds', () => {
      createGauge({
        gauge: {
          thresholds: [
            { value: 75, color: '#ff3b30' },
            { value: 60, color: '#ffcc00' }
          ]
        }
      });
      const dataset: Record<string, any> = component.chart.config.data.datasets[0];
      component.dataChanged({ cpu: 40 });
      expect(dataset.backgroundColor[0]).toBe('#4cd964');
      component.dataChanged({ cpu: 60 });
      expect(dataset.backgroundColor[0]).toBe('#ffcc00');
      component.dataChanged({ cpu: 80 });
      expect(dataset.backgroundColor[0]).toBe('#ff3b30');
      expect(dataset.backgroundColor[1]).toBe('transparent');
    });

    it('should color each dataset by its own value', () => {
      createGauge({
        dataConfig: [
          { label: 'CPU', prop: 'cpu', backgroundColor: '#4cd964' },
          { label: 'Disk', prop: 'disk', backgroundColor: '#5ac8fa' }
        ],
        gauge: {
          thresholds: [{ value: 50, color: '#ff3b30' }]
        }
      });
      const datasets: Array<Record<string, any>> = component.chart.config.data.datasets;
      component.dataChanged({ cpu: 70, disk: 30 });
      expect(datasets[0].backgroundColor[0]).toBe('#ff3b30');
      expect(datasets[1].backgroundColor[1]).toBe('#5ac8fa');
    });

    it('should draw without conic gradient support', () => {
      createGauge();
      expect(() => component.dataChanged({ cpu: 70 })).not.toThrow();
    });
  });

  describe('chartStylePlugin', () => {
    // A doughnut with an outer radius of 70 and an inner radius of 58,
    // so the ring is 12 pixels wide.
    const createArc = (startAngle: number, endAngle: number) => ({
      x: 100,
      y: 100,
      startAngle,
      endAngle,
      circumference: endAngle - startAngle,
      outerRadius: 70,
      innerRadius: 58,
      options: { backgroundColor: '#000000' },
      draw: jest.fn()
    });

    const draw = (
      arcs: Array<Record<string, any>>,
      colors: Array<string>,
      options: Record<string, any> = { gauge: false },
      hook: 'beforeDatasetsDraw' | 'afterDatasetsDraw' = 'afterDatasetsDraw',
      config: Record<string, any> = { rotation: 0, circumference: 360 }
    ) => {
      const ctx: Record<string, any> = {
        save: jest.fn(),
        restore: jest.fn(),
        beginPath: jest.fn(),
        arc: jest.fn(),
        fill: jest.fn(),
        stroke: jest.fn(),
        createConicGradient: jest.fn(() => ({ addColorStop: jest.fn() }))
      };
      const chart = {
        ctx,
        config: { options: config },
        data: { datasets: [{ backgroundColor: colors }] },
        getDatasetMeta: () => ({ data: arcs })
      };
      (chartStylePlugin[hook] as any)(chart, {}, options);
      return ctx;
    };

    it('should keep a short arc in a solid color', () => {
      // A tiny value, enlarged to a dot as long as the ring is wide.
      const tiny = createArc(0, 12 / 64);
      const large = createArc(12 / 64, 2 * Math.PI);
      draw([tiny, large], ['#f44336', '#4cd964']);
      // The large segment is redrawn with a gradient, the tiny one is not.
      expect(large.draw).toHaveBeenCalled();
      expect(tiny.draw).not.toHaveBeenCalled();
    });

    it('should draw segments as arcs', () => {
      const used = createArc(0, 0.6 * Math.PI);
      const free = createArc(0.6 * Math.PI, 2 * Math.PI);
      const ctx = draw([used, free], ['#f44336', '#4cd964']);
      expect(used.draw).toHaveBeenCalled();
      expect(free.draw).toHaveBeenCalled();
      expect(ctx.arc).not.toHaveBeenCalled();
    });

    it('should draw the full gauge track underneath the value', () => {
      // A 270° gauge in the middle of its sweep-in animation: the
      // elements only cover a part of the gauge so far.
      const start = -1.25 * Math.PI;
      const value = createArc(start, start + 0.2 * Math.PI);
      const rest = createArc(start + 0.2 * Math.PI, start + 0.5 * Math.PI);
      const ctx = draw(
        [value, rest],
        ['#4cd964', 'transparent'],
        { gauge: true, getTrackColor: () => '#0000001a' },
        'beforeDatasetsDraw',
        { rotation: -135, circumference: 270 }
      );
      expect(ctx.strokeStyle).toBe('#0000001a');
      expect(ctx.lineWidth).toBe(12);
      expect(ctx.lineCap).toBe('round');
      // The track is drawn from the start to the end of the gauge,
      // inset by the round caps, regardless of the animation.
      const inset = 6 / 64;
      const [x, y, radius, from, to] = ctx.arc.mock.calls[0];
      expect([x, y, radius]).toEqual([100, 100, 64]);
      expect(from).toBeCloseTo(start + inset);
      expect(to).toBeCloseTo(start + 1.5 * Math.PI - inset);
      expect(ctx.stroke).toHaveBeenCalled();
    });

    it('should draw a full circle as the doughnut track', () => {
      const ctx = draw(
        [createArc(0, 0.6 * Math.PI), createArc(0.6 * Math.PI, 2 * Math.PI)],
        ['#f44336', '#4cd964'],
        { gauge: false, getTrackColor: () => '#ffffff1f' },
        'beforeDatasetsDraw'
      );
      expect(ctx.strokeStyle).toBe('#ffffff1f');
      expect(ctx.lineWidth).toBe(12);
      expect(ctx.arc).toHaveBeenCalledWith(100, 100, 64, -0.5 * Math.PI, 1.5 * Math.PI);
    });
  });
});
