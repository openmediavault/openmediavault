import { ComponentFixture, TestBed } from '@angular/core/testing';

import { ComponentsModule } from '~/app/core/components/components.module';
import { WelcomePanelComponent } from '~/app/core/components/welcome-panel/welcome-panel.component';
import { TestingModule } from '~/app/testing.module';

describe('WelcomePanelComponent', () => {
  let component: WelcomePanelComponent;
  let fixture: ComponentFixture<WelcomePanelComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [WelcomePanelComponent],
      imports: [ComponentsModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(WelcomePanelComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
