import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { ToastrModule } from 'ngx-toastr';

import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { TextPageComponent } from '~/app/core/components/intuition/text-page/text-page.component';

describe('TextareaPageComponent', () => {
  let component: TextPageComponent;
  let fixture: ComponentFixture<TextPageComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [
          HttpClientTestingModule,
          IntuitionModule,
          RouterTestingModule,
          ToastrModule.forRoot()
        ]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(TextPageComponent);
    component = fixture.componentInstance;
    component.config = {};
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
