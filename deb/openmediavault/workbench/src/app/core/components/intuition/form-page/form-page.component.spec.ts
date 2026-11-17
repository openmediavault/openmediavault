import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { ActivatedRoute, Router } from '@angular/router';
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

  describe('returnUrl', () => {
    let navigateByUrl: jest.SpyInstance;

    beforeEach(() => {
      navigateByUrl = jest.spyOn(TestBed.inject(Router), 'navigateByUrl').mockResolvedValue(true);
      component.config.buttons = [{ template: 'cancel', execute: { type: 'url', url: '/foo' } }];
    });

    const setReturnUrl = (returnUrl: string) => {
      (TestBed.inject(ActivatedRoute).snapshot as any).queryParams = { returnUrl };
    };

    it('should navigate to the return URL', () => {
      setReturnUrl('/bar');
      component.onButtonClick(component.config.buttons[0]);
      expect(navigateByUrl).toHaveBeenCalledWith('/bar');
    });

    it.each(['/download?service=A&method=b', 'download?service=A&method=b', '/download'])(
      'should ignore the download return URL %s',
      (returnUrl) => {
        setReturnUrl(returnUrl);
        component.onButtonClick(component.config.buttons[0]);
        expect(navigateByUrl).toHaveBeenCalledTimes(1);
        expect(navigateByUrl).toHaveBeenCalledWith('/foo');
      }
    );
  });
});
