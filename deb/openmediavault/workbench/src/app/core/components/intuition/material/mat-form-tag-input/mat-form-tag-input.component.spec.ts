import { ComponentFixture, TestBed } from '@angular/core/testing';

import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { MatFormTagInputComponent } from '~/app/core/components/intuition/material/mat-form-tag-input/mat-form-tag-input.component';

describe('MatFormTagInputComponent', () => {
  let component: MatFormTagInputComponent;
  let fixture: ComponentFixture<MatFormTagInputComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [MatFormTagInputComponent],
      imports: [IntuitionModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(MatFormTagInputComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
