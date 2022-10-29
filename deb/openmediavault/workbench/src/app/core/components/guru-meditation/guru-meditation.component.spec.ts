import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { TranslateModule } from '@ngx-translate/core';

import { GuruMeditationComponent } from '~/app/core/components/guru-meditation/guru-meditation.component';
import { TestingModule } from '~/app/testing.module';

describe('GuruMeditationComponent', () => {
  let component: GuruMeditationComponent;
  let fixture: ComponentFixture<GuruMeditationComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      declarations: [GuruMeditationComponent],
      imports: [TestingModule, TranslateModule.forRoot()]
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
