import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';

import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { RrdPageComponent } from '~/app/core/components/intuition/rrd-page/rrd-page.component';

describe('RrdPageComponent', () => {
  let component: RrdPageComponent;
  let fixture: ComponentFixture<RrdPageComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [HttpClientTestingModule, IntuitionModule, RouterTestingModule]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(RrdPageComponent);
    component = fixture.componentInstance;
    component.config = {
      graphs: []
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
