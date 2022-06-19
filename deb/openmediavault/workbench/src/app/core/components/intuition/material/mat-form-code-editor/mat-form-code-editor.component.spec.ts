import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { MatFormButtonComponent } from '~/app/core/components/intuition/material/mat-form-button/mat-form-button.component';

describe('MatFormButtonComponent', () => {
  let component: MatFormButtonComponent;
  let fixture: ComponentFixture<MatFormButtonComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      declarations: [MatFormButtonComponent]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(MatFormButtonComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
