import { CronTaskDatatablePageComponent } from '~/app/pages/system/cron/cron-task-datatable-page.component';

describe('CronTaskDatatablePageComponent', () => {
  it('should enable client-side search for scheduled tasks', () => {
    const component = new CronTaskDatatablePageComponent();

    expect(component.config.hasSearchField).toBe(true);
    expect(component.config.remoteSearching).not.toBe(true);
  });
});
