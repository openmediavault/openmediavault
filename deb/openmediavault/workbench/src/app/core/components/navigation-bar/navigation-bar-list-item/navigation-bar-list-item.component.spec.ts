/* eslint-disable max-len */
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { Event, NavigationEnd, Router } from '@angular/router';
import { Subject } from 'rxjs';

import { ComponentsModule } from '~/app/core/components/components.module';
import { NavigationBarListItemComponent } from '~/app/core/components/navigation-bar/navigation-bar-list-item/navigation-bar-list-item.component';
import { TestingModule } from '~/app/testing.module';

describe('NavbarListItemComponent', () => {
  let component: NavigationBarListItemComponent;
  let fixture: ComponentFixture<NavigationBarListItemComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [ComponentsModule, TestingModule]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NavigationBarListItemComponent);
    component = fixture.componentInstance;
    component.item = { text: 'foo' };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  describe('active state', () => {
    const setUrl = (url: string) => {
      Object.defineProperty(TestBed.inject(Router), 'url', { configurable: true, value: url });
    };
    const navigate = (url: string) => {
      setUrl(url);
      (TestBed.inject(Router).events as Subject<Event>).next(new NavigationEnd(1, url, url));
    };

    beforeEach(() => {
      component.item = { text: 'PV', url: '/k8s/persistentvolume' };
    });

    it('should be active on the exact URL', () => {
      navigate('/k8s/persistentvolume');
      expect(component.item.active).toBe(true);
    });

    it('should be active on a sub page', () => {
      navigate('/k8s/persistentvolume/get/foo');
      expect(component.item.active).toBe(true);
    });

    it('should ignore the query string and fragment', () => {
      navigate('/k8s/persistentvolume?namespace=foo');
      expect(component.item.active).toBe(true);
      navigate('/k8s/persistentvolume#bar');
      expect(component.item.active).toBe(true);
    });

    it('should not be active if another item only shares the URL prefix', () => {
      navigate('/k8s/persistentvolumeclaim');
      expect(component.item.active).toBe(false);
    });

    it('should not be active on sub pages of an item sharing the URL prefix', () => {
      navigate('/k8s/persistentvolumeclaim/get/foo');
      expect(component.item.active).toBe(false);
    });

    it('should handle an item URL with a trailing slash', () => {
      component.item.url = '/k8s/ingress/';
      navigate('/k8s/ingress');
      expect(component.item.active).toBe(true);
      navigate('/k8s/ingressroute');
      expect(component.item.active).toBe(false);
    });

    it('should not be active if the item has no URL', () => {
      component.item = { text: 'group' };
      navigate('/k8s/persistentvolume');
      expect(component.item.active).toBe(false);
    });

    it('should only be active on the exact URL if the item has children', () => {
      component.item.children = [{ text: 'child' }];
      navigate('/k8s/persistentvolume/get/foo');
      expect(component.item.active).toBe(false);
      navigate('/k8s/persistentvolume');
      expect(component.item.active).toBe(true);
    });

    it('should not be active or expanded initially if another item shares the URL prefix', () => {
      setUrl('/k8s/persistentvolumeclaim');
      component.ngOnInit();
      expect(component.item.active).toBe(false);
      expect(component.item.expanded).toBe(false);
    });

    it('should be expanded initially if the URL is below the item', () => {
      setUrl('/k8s/persistentvolume/get/foo');
      component.ngOnInit();
      expect(component.item.expanded).toBe(true);
    });
  });
});
