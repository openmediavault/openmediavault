// This file is part of OpenMediaVault.
//
// @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
// @author    Volker Theile <volker.theile@openmediavault.org>
// @copyright Copyright (c) 2009-2025 Volker Theile
//
// OpenMediaVault is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// OpenMediaVault is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
/* eslint-disable @typescript-eslint/member-ordering */
import {
  ChangeDetectionStrategy,
  Component,
  ElementRef,
  HostListener,
  Input,
  OnDestroy,
  OnInit,
  ViewChild
} from '@angular/core';
import * as _ from 'lodash';
import { Subscription, timer } from 'rxjs';

type Column = {
  x: number;
  stackHeight: number;
  stackCounter: number;
  color: string;
};

/**
 * Inspired by https://gist.github.com/pingpoli/39dd5f19a934be466941ad26bedbffac#file-matrix-min-html
 */
@Component({
  selector: 'omv-green-rain',
  templateUrl: './green-rain.component.html',
  styleUrls: ['./green-rain.component.scss'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class GreenRainComponent implements OnInit, OnDestroy {
  @ViewChild('canvas', { static: true })
  canvas: ElementRef<HTMLCanvasElement>;

  @Input()
  delay?: number = 0; // milliseconds

  @Input()
  fadeFactor?: number = 0.05;

  @Input()
  interval?: number = 75; // milliseconds

  @Input()
  primaryColor?: string = '#4cd964';

  @Input()
  secondaryColor?: string = '#33ff33';

  private columns: Column[] = [];
  private context: CanvasRenderingContext2D;
  private tileSize: number;
  private fontSize: number;
  private fontFamily: string;
  private timerSubscription: Subscription;
  private initialDelayComplete = false;
  private prefersReducedMotion = false;

  private get width(): number {
    const element = this.canvas.nativeElement as HTMLCanvasElement;
    return element.width;
  }

  private get height(): number {
    const element = this.canvas.nativeElement as HTMLCanvasElement;
    return element.height;
  }

  constructor() {
    this.prefersReducedMotion = window.matchMedia?.('(prefers-reduced-motion: reduce)').matches;
  }

  @HostListener('click', ['$event'])
  onClick() {
    if (!this.prefersReducedMotion && this.initialDelayComplete) {
      this.restart(this.delay);
    }
  }

  @HostListener('window:resize', ['$event'])
  onResize() {
    if (!this.prefersReducedMotion && this.initialDelayComplete) {
      this.restart(200);
    }
  }

  ngOnInit(): void {
    if (this.prefersReducedMotion) {
      return;
    }

    const element = this.canvas.nativeElement as HTMLCanvasElement;
    const styles = window.getComputedStyle(element);

    this.initialDelayComplete = false;
    this.context = element.getContext('2d');
    this.fontSize = parseInt(styles.fontSize, 10);
    this.fontFamily = styles.fontFamily;
    this.tileSize = this.fontSize + 2;

    this.init();
    this.start(this.delay);
  }

  ngOnDestroy(): void {
    this.stop();
  }

  private init(): void {
    const element = this.canvas.nativeElement as HTMLCanvasElement;

    element.style.width = '100%';
    element.style.height = '100%';
    element.width = element.offsetWidth;
    element.height = element.offsetHeight;

    this.columns = [];
    for (let i = 0; i < this.width / this.tileSize; i++) {
      this.columns.push({
        x: i * this.tileSize,
        stackCounter: 0,
        stackHeight: this.getRandomStackHeight(),
        color: this.getRandomColor()
      });
    }
  }

  private start(delay: number = 0): void {
    this.timerSubscription = timer(delay, this.interval).subscribe(() => {
      this.initialDelayComplete = true;
      this.draw();
    });
  }

  private stop(): void {
    if (this.timerSubscription) {
      this.timerSubscription.unsubscribe();
    }
  }

  private restart(delay: number = 0) {
    this.stop();
    this.init();
    this.start(delay);
  }

  private draw(): void {
    this.context.font = `${this.fontSize}px ${this.fontFamily}`;
    this.context.fillStyle = `rgba(0, 0, 0, ${this.fadeFactor})`;
    this.context.fillRect(0, 0, this.width, this.height);

    this.columns.forEach((column: Column) => {
      this.context.fillStyle = column.color;
      this.context.fillText(
        this.getRandomText(),
        column.x,
        column.stackCounter * this.tileSize + this.tileSize
      );

      if (++column.stackCounter >= column.stackHeight) {
        _.merge(column, {
          stackCounter: 0,
          stackHeight: this.getRandomStackHeight(),
          color: this.getRandomColor()
        });
      }
    });
  }

  private getRandomStackHeight(): number {
    const maxStackHeight: number = _.ceil(this.height / this.tileSize);
    return _.random(10, maxStackHeight);
  }

  private getRandomText(): string {
    return String.fromCharCode(_.random(33, 126));
  }

  /**
   * Get a random color with a distribution of 9:1 (primary
   * to secondary).
   *
   * @private
   */
  private getRandomColor(): string {
    return _.sample([
      this.primaryColor,
      this.primaryColor,
      this.primaryColor,
      this.primaryColor,
      this.primaryColor,
      this.primaryColor,
      this.primaryColor,
      this.primaryColor,
      this.primaryColor,
      this.secondaryColor
    ]);
  }
}
