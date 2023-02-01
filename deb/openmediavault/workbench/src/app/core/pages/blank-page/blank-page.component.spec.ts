import { ComponentFixture, TestBed } from '@angular/core/testing';

import { BlankPageComponent } from '~/app/core/pages/blank-page/blank-page.component';
import { PagesModule } from '~/app/core/pages/pages.module';
import { TestingModule } from '~/app/testing.module';

describe('BlankPageComponent', () => {
  let component: BlankPageComponent;
  let fixture: ComponentFixture<BlankPageComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [PagesModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(BlankPageComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
