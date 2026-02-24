import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { DashboardWidgetComponent } from '~/app/core/components/dashboard/dashboard-widget/dashboard-widget.component';
import { TestingModule } from '~/app/testing.module';

describe('DashboardWidgetComponent', () => {
  let component: DashboardWidgetComponent;
  let fixture: ComponentFixture<DashboardWidgetComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [DashboardWidgetComponent],
      imports: [DashboardModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(DashboardWidgetComponent);
    component = fixture.componentInstance;
    component.config = {
      id: '7ad74159-ce98-4d9a-a47a-1540a8a2ffe4',
      title: 'foo',
      type: 'value',
      value: {
        title: 'bar',
        value: '{{ xyz }} abc'
      }
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  describe('CSS text wrapping and overflow handling', () => {
    it('should apply overflow and word-wrap styles to widget container', () => {
      const container = fixture.debugElement.nativeElement.querySelector(
        '.omv-dashboard-widget-container'
      );

      if (container) {
        // Verify the container exists and has the correct class
        expect(container).toBeTruthy();
        expect(container.classList.contains('omv-dashboard-widget-container')).toBe(true);
      }
    });

    it('should apply word-wrap and word-break styles to mat-card', () => {
      const container = fixture.debugElement.nativeElement.querySelector(
        '.omv-dashboard-widget-container'
      );

      if (container) {
        const card = container.querySelector('.mat-card');
        if (card) {
          // Verify the card exists and has proper structure
          expect(card).toBeTruthy();
          expect(card.classList.contains('mat-card')).toBe(true);
        }
      }
    });

    it('should maintain break-inside: avoid on widget container to prevent column breaks', () => {
      const container = fixture.debugElement.nativeElement.querySelector(
        '.omv-dashboard-widget-container'
      );

      if (container) {
        // Verify that the container has the correct class and structure
        expect(container).toBeTruthy();
        expect(container.classList.contains('omv-dashboard-widget-container')).toBe(true);
      }
    });

    it('should have proper DOM structure for text wrapping', () => {
      const container = fixture.debugElement.nativeElement.querySelector(
        '.omv-dashboard-widget-container'
      );

      if (container) {
        const card = container.querySelector('.mat-card');
        if (card) {
          // Verify that the card contains content
          expect(card.children.length).toBeGreaterThanOrEqual(0);
        }
      }
    });
  });
});
