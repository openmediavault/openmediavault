import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';

import { LimnUiModule } from '~/app/core/components/limn-ui/limn-ui.module';
import { RrdPageComponent } from '~/app/core/components/limn-ui/rrd-page/rrd-page.component';

describe('RrdPageComponent', () => {
  let component: RrdPageComponent;
  let fixture: ComponentFixture<RrdPageComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [HttpClientTestingModule, LimnUiModule, RouterTestingModule]
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
