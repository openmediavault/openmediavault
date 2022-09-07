import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { WidgetTextComponent } from '~/app/core/components/dashboard/widgets/widget-text/widget-text.component';
import { TestingModule } from '~/app/testing.module';

describe('WidgetTextComponent', () => {
  let component: WidgetTextComponent;
  let fixture: ComponentFixture<WidgetTextComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [WidgetTextComponent],
      imports: [DashboardModule, TestingModule]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(WidgetTextComponent);
    component = fixture.componentInstance;
    component.config = {
      id: 'fd896a2c-5287-49d7-b773-e25c62071540',
      type: 'text',
      title: 'foo'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
