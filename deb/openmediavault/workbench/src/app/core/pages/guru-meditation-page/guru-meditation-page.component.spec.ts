import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { TranslateModule } from '@ngx-translate/core';

import { GuruMeditationPageComponent } from '~/app/core/pages/guru-meditation-page/guru-meditation-page.component';
import { PagesModule } from '~/app/core/pages/pages.module';

describe('NotFoundPageComponent', () => {
  let component: GuruMeditationPageComponent;
  let fixture: ComponentFixture<GuruMeditationPageComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [PagesModule, RouterTestingModule, TranslateModule.forRoot()]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(GuruMeditationPageComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
