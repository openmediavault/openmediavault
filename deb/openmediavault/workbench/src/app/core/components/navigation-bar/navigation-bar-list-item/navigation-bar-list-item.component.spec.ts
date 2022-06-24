/* eslint-disable max-len */
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { ComponentsModule } from '~/app/core/components/components.module';
import { NavigationBarListItemComponent } from '~/app/core/components/navigation-bar/navigation-bar-list-item/navigation-bar-list-item.component';
import { TestingModule } from '~/app/testing.module';

describe('NavbarListItemComponent', () => {
  let component: NavigationBarListItemComponent;
  let fixture: ComponentFixture<NavigationBarListItemComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [ComponentsModule, TestingModule]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NavigationBarListItemComponent);
    component = fixture.componentInstance;
    component.item = { text: 'foo' };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
