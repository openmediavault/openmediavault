import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { RrdPageComponent } from '~/app/core/components/intuition/rrd-page/rrd-page.component';
import { TestingModule } from '~/app/testing.module';

describe('RrdPageComponent', () => {
  let component: RrdPageComponent;
  let fixture: ComponentFixture<RrdPageComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [IntuitionModule, TestingModule]
    }).compileComponents();
  }));

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
