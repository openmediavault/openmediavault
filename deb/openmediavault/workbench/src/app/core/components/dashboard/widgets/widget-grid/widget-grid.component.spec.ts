import { ComponentFixture, TestBed } from '@angular/core/testing';
import { TranslateModule } from '@ngx-translate/core';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { WidgetGridComponent } from '~/app/core/components/dashboard/widgets/widget-grid/widget-grid.component';
import { TestingModule } from '~/app/testing.module';

describe('WidgetGridComponent', () => {
  let component: WidgetGridComponent;
  let fixture: ComponentFixture<WidgetGridComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [DashboardModule, TestingModule, TranslateModule.forRoot()]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(WidgetGridComponent);
    component = fixture.componentInstance;
    component.config = {
      id: '2f06f7e5-b34f-47f7-a953-5eba2f386139',
      type: 'grid',
      title: 'bar'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
