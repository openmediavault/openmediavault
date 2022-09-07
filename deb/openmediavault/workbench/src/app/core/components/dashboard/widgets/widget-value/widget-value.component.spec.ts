import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { WidgetValueComponent } from '~/app/core/components/dashboard/widgets/widget-value/widget-value.component';
import { TestingModule } from '~/app/testing.module';

describe('WidgetValueComponent', () => {
  let component: WidgetValueComponent;
  let fixture: ComponentFixture<WidgetValueComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [WidgetValueComponent],
      imports: [DashboardModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(WidgetValueComponent);
    component = fixture.componentInstance;
    component.config = {
      id: '5a388783-c349-4032-9747-2482563c44dd',
      type: 'value',
      title: 'bar',
      value: {
        title: '{{ foo }} baz',
        value: 'foo {{ xyz }} bar'
      }
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
