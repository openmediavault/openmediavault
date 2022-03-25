import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { GuruMeditationComponent } from '~/app/core/components/guru-meditation/guru-meditation.component';

describe('GuruMeditationComponent', () => {
  let component: GuruMeditationComponent;
  let fixture: ComponentFixture<GuruMeditationComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      declarations: [GuruMeditationComponent]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(GuruMeditationComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
