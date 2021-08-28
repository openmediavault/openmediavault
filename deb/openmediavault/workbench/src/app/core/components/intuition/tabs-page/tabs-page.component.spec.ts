import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';

import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { TabsPageComponent } from '~/app/core/components/intuition/tabs-page/tabs-page.component';

describe('TabsPageComponent', () => {
  let component: TabsPageComponent;
  let fixture: ComponentFixture<TabsPageComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [IntuitionModule, RouterTestingModule]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(TabsPageComponent);
    component = fixture.componentInstance;
    component.config = {
      tabs: []
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
