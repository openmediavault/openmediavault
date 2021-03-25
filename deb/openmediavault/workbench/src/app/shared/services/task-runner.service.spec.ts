import { HttpClientTestingModule } from '@angular/common/http/testing';
import { TestBed } from '@angular/core/testing';

import { TaskRunnerService } from '~/app/shared/services/task-runner.service';

describe('TaskRunnerService', () => {
  beforeEach(() =>
    TestBed.configureTestingModule({
      imports: [HttpClientTestingModule]
    })
  );

  it('should be created', () => {
    const service: TaskRunnerService = TestBed.inject(TaskRunnerService);
    expect(service).toBeTruthy();
  });
});
