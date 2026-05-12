import { TestBed } from '@angular/core/testing';

import { TaskRunnerService } from '~/app/shared/services/task-runner.service';
import { TestingModule } from '~/app/testing.module';

describe('TaskRunnerService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [TestingModule]
    })
  );

  it('should be created', () => {
    const service: TaskRunnerService = TestBed.inject(TaskRunnerService);
    expect(service).toBeTruthy();
  });
});
