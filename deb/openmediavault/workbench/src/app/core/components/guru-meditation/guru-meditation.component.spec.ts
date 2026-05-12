import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { ComponentsModule } from '~/app/core/components/components.module';
import { GuruMeditationComponent } from '~/app/core/components/guru-meditation/guru-meditation.component';
import { TestingModule } from '~/app/testing.module';

describe('GuruMeditationComponent', () => {
  let component: GuruMeditationComponent;
  let fixture: ComponentFixture<GuruMeditationComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      declarations: [GuruMeditationComponent],
      imports: [ComponentsModule, TestingModule]
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
