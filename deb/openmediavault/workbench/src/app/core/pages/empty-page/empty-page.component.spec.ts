import { ComponentFixture, TestBed } from '@angular/core/testing';

import { EmptyPageComponent } from '~/app/core/pages/empty-page/empty-page.component';
import { PagesModule } from '~/app/core/pages/pages.module';
import { TestingModule } from '~/app/testing.module';

describe('EmptyPageComponent', () => {
  let component: EmptyPageComponent;
  let fixture: ComponentFixture<EmptyPageComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [PagesModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(EmptyPageComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
