import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { ToastrModule } from 'ngx-toastr';
import { of } from 'rxjs';

import { FormPageComponent } from '~/app/core/components/intuition/form-page/form-page.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { RpcService } from '~/app/shared/services/rpc.service';
import { TestingModule } from '~/app/testing.module';

describe('FormPageComponent', () => {
  let component: FormPageComponent;
  let fixture: ComponentFixture<FormPageComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [IntuitionModule, TestingModule, ToastrModule.forRoot()]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FormPageComponent);
    component = fixture.componentInstance;
    component.config = {
      fields: []
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  it('should format the POST request properties on submit using the form values', () => {
    const request = jest.spyOn(TestBed.inject(RpcService), 'request').mockReturnValue(of({}));
    component.config.request = {
      service: 'Foo',
      post: { method: 'set{{ name }}', params: { copy: '{{ foo }}' } }
    };
    component.pageContextService.set({ name: 'A' });
    jest.spyOn(component, 'getFormValues').mockReturnValue({ foo: 'bar' });
    component.onButtonClick({ template: 'submit' });
    // The method uses the page context, the parameters the form values.
    expect(request).toHaveBeenCalledWith('Foo', 'setA', { foo: 'bar', copy: 'bar' });
    // The tokens must be kept in the configuration.
    expect(component.config.request.post.params).toEqual({ copy: '{{ foo }}' });
  });
});
