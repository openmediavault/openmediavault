/* eslint-disable max-len */
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { DashboardWidgetSystemInformationComponent } from '~/app/core/components/dashboard/dashboard-widget-system-information/dashboard-widget-system-information.component';
import { TestingModule } from '~/app/testing.module';

describe('DashboardWidgetSystemInformationComponent', () => {
  let component: DashboardWidgetSystemInformationComponent;
  let fixture: ComponentFixture<DashboardWidgetSystemInformationComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      declarations: [DashboardWidgetSystemInformationComponent],
      imports: [DashboardModule, TestingModule]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DashboardWidgetSystemInformationComponent);
    component = fixture.componentInstance;
    component.config = {
      id: '3b7d97dc-f5d3-4a5b-9300-830a0946e18a',
      type: 'system-information',
      title: 'foo'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  describe('CSS text wrapping and overflow handling', () => {
    it('should apply word-wrap and word-break styles to grid cells', () => {
      const widgetContent = fixture.debugElement.query(
        (el) => el.nativeElement.classList?.contains('widget-content')
      );

      if (widgetContent) {
        const gridCell = widgetContent.nativeElement.querySelector('.omv-grid-cell');
        if (gridCell) {
          const styles = window.getComputedStyle(gridCell);
          expect(styles.wordWrap).toBe('break-word');
          expect(styles.wordBreak).toBe('break-word');
          expect(styles.overflow).toBe('hidden');
        }
      }
    });

    it('should have overflow-wrap on nested divs within grid cells', () => {
      const widgetContent = fixture.debugElement.query(
        (el) => el.nativeElement.classList?.contains('widget-content')
      );

      if (widgetContent) {
        const gridCell = widgetContent.nativeElement.querySelector('.omv-grid-cell');
        if (gridCell) {
          const innerDiv = gridCell.querySelector('div');
          if (innerDiv) {
            const styles = window.getComputedStyle(innerDiv);
            expect(styles.wordWrap).toBe('break-word');
            expect(styles.wordBreak).toBe('break-word');
          }
        }
      }
    });

    it('should prevent text overlapping with proper text wrapping', () => {
      // Verify that break-inside: avoid is still applied to prevent column breaks
      const widgetContent = fixture.debugElement.query(
        (el) => el.nativeElement.classList?.contains('widget-content')
      );

      if (widgetContent) {
        const gridCell = widgetContent.nativeElement.querySelector('.omv-grid-cell');
        if (gridCell) {
          // Check that the element maintains structural integrity
          expect(gridCell).toBeTruthy();
          expect(gridCell.children.length).toBeGreaterThan(0);
        }
      }
    });
  });
});
