import { ComponentFixture, TestBed } from '@angular/core/testing';

import { GreenRainComponent } from '~/app/core/components/green-rain/green-rain.component';

describe('GreenRainComponent', () => {
  let component: GreenRainComponent;
  let fixture: ComponentFixture<GreenRainComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [GreenRainComponent]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(GreenRainComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
