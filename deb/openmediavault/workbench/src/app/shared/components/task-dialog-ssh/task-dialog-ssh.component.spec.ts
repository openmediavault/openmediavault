import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { MAT_DIALOG_DATA } from '@angular/material/dialog';

import { ComponentsModule } from '~/app/shared/components/components.module';
import { TaskDialogSshComponent } from '~/app/shared/components/task-dialog-ssh/task-dialog-ssh.component';
import { TestingModule } from '~/app/testing.module';

describe('TaskDialogSshComponent', () => {
  let component: TaskDialogSshComponent;
  let fixture: ComponentFixture<TaskDialogComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [ComponentsModule, TestingModule],
      providers: [{ provide: MAT_DIALOG_DATA, useValue: {} }]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TaskDialogSshComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
