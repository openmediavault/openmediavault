/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
import { Component, ElementRef, OnDestroy, OnInit } from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';
import * as _ from 'lodash';

@Component({
  selector: 'omv-guru-meditation-page',
  templateUrl: './guru-meditation-page.component.html',
  styleUrls: ['./guru-meditation-page.component.scss']
})
export class GuruMeditationPageComponent implements OnInit, OnDestroy {
  message: string;

  constructor(
    private activatedRoute: ActivatedRoute,
    private elementRef: ElementRef,
    private router: Router
  ) {
    this.message = _.get(this.activatedRoute, 'routeConfig.data.message');
  }

  ngOnInit(): void {
    this.elementRef.nativeElement.addEventListener('click', this.onClick.bind(this));
  }

  ngOnDestroy(): void {
    this.elementRef.nativeElement.removeEventListener('click', this.onClick);
  }

  private onClick() {
    this.router.navigate(['/']);
  }
}
