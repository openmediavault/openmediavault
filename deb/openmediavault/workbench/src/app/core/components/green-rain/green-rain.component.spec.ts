import { ComponentFixture, TestBed } from '@angular/core/testing';

import { ComponentsModule } from '~/app/core/components/components.module';
import { GreenRainComponent } from '~/app/core/components/green-rain/green-rain.component';
import { TestingModule } from '~/app/testing.module';

describe('GreenRainComponent', () => {
  let component: GreenRainComponent;
  let fixture: ComponentFixture<GreenRainComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [GreenRainComponent],
      imports: [ComponentsModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(GreenRainComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
    // eslint-disable-next-line @typescript-eslint/dot-notation
    expect(component['prefersReducedMotion']).toBeFalsy();
  });
});
