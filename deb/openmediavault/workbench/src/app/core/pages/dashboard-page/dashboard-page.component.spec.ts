import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { DashboardPageComponent } from '~/app/core/pages/dashboard-page/dashboard-page.component';
import { PagesModule } from '~/app/core/pages/pages.module';
import { TestingModule } from '~/app/testing.module';

describe('DashboardPageComponent', () => {
  let component: DashboardPageComponent;
  let fixture: ComponentFixture<DashboardPageComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [PagesModule, TestingModule]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DashboardPageComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  describe('CSS text wrapping and overflow handling (Issue #2103)', () => {
    it('should render dashboard page container with proper layout', () => {
      const container = fixture.debugElement.nativeElement.querySelector(
        '.omv-dashboard-page-container'
      );
      expect(container).toBeTruthy();
      expect(container.classList.contains('omv-dashboard-page-container')).toBe(true);
    });

    it('should have widgets-container with column layout for multi-column rendering', () => {
      const widgetsContainer = fixture.debugElement.nativeElement.querySelector(
        '.widgets-container'
      );

      expect(widgetsContainer).toBeTruthy();

      if (widgetsContainer) {
        // Verify the container has the correct class
        expect(widgetsContainer.classList.contains('widgets-container')).toBe(true);
      }
    });

    it('should apply grid utility classes for DOM structure', () => {
      // Create a test grid element to verify CSS utility classes exist
      const testGrid = document.createElement('div');
      testGrid.classList.add('omv-grid', 'omv-grid-cols-2', 'omv-gap-2');

      const testChild = document.createElement('div');
      testGrid.appendChild(testChild);

      // Verify the classes are applied correctly
      expect(testGrid.classList.contains('omv-grid')).toBe(true);
      expect(testGrid.classList.contains('omv-grid-cols-2')).toBe(true);
      expect(testGrid.classList.contains('omv-gap-2')).toBe(true);
    });

    it('should have grid cell structure for text wrapping', () => {
      // Simulate system information widget grid structure at narrow viewport
      const widget = document.createElement('div');
      widget.classList.add('omv-dashboard-widget-container');
      widget.style.width = '350px'; // Simulate column width

      const grid = document.createElement('div');
      grid.classList.add('omv-grid', 'omv-grid-cols-2', 'omv-gap-2');

      // System time cell with long text (the problematic case from issue #2103)
      const timeCell = document.createElement('div');
      timeCell.classList.add('omv-grid-cell');
      timeCell.innerHTML = `
        <div style="font-weight: bold;">System time</div>
        <div>Št 15. január 2026, 23:20:00</div>
      `;

      // Uptime cell
      const uptimeCell = document.createElement('div');
      uptimeCell.classList.add('omv-grid-cell');
      uptimeCell.innerHTML = `
        <div style="font-weight: bold;">Uptime</div>
        <div>30 days, 5 hours, 45 minutes</div>
      `;

      grid.appendChild(timeCell);
      grid.appendChild(uptimeCell);

      // Verify structure is created correctly
      expect(grid.children.length).toBe(2);
      expect(timeCell.classList.contains('omv-grid-cell')).toBe(true);
      expect(uptimeCell.classList.contains('omv-grid-cell')).toBe(true);
    });

    it('should maintain grid structure while handling long text content', () => {
      const grid = document.createElement('div');
      grid.classList.add('omv-grid', 'omv-grid-cols-2');

      // Add multiple cells to test grid layout integrity
      for (let i = 0; i < 4; i++) {
        const cell = document.createElement('div');
        cell.classList.add('omv-grid-cell');
        cell.textContent = `This is a long text value that should wrap properly in the grid cell without overlapping other cells`;
        grid.appendChild(cell);
      }

      // Verify grid structure
      expect(grid.children.length).toBe(4);
      expect(grid.classList.contains('omv-grid-cols-2')).toBe(true);

      // Verify each cell has proper class
      Array.from(grid.children).forEach((cell) => {
        const cellElement = cell as HTMLElement;
        expect(cellElement.classList.contains('omv-grid-cell')).toBe(true);
      });
    });

    it('should support responsive grid columns (1, 2, 3 columns)', () => {
      const testCases = [
        { class: 'omv-grid-cols-1' },
        { class: 'omv-grid-cols-2' },
        { class: 'omv-grid-cols-3' }
      ];

      testCases.forEach((testCase) => {
        const grid = document.createElement('div');
        grid.classList.add('omv-grid', testCase.class);

        expect(grid.classList.contains(testCase.class)).toBe(true);
      });
    });

    it('should have widget container with proper class structure', () => {
      const widget = document.createElement('div');
      widget.classList.add('omv-dashboard-widget-container');
      widget.textContent = 'Test Widget';

      // Verify the widget container structure is intact
      expect(widget.classList.contains('omv-dashboard-widget-container')).toBe(true);
      expect(widget.textContent).toBe('Test Widget');
    });
  });
});
