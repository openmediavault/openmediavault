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
