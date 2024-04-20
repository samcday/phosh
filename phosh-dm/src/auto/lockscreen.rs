// This file was generated by gir (https://github.com/gtk-rs/gir)
// from ..
// from ../gir-files
// DO NOT EDIT

use crate::{LayerSurface,LockscreenPage};
use glib::{prelude::*,signal::{connect_raw, SignalHandlerId},translate::*};
use std::{boxed::Box as Box_};

glib::wrapper! {
    #[doc(alias = "PhoshLockscreen")]
    pub struct Lockscreen(Object<ffi::PhoshLockscreen, ffi::PhoshLockscreenClass>) @extends LayerSurface;

    match fn {
        type_ => || ffi::phosh_lockscreen_get_type(),
    }
}

impl Lockscreen {
        pub const NONE: Option<&'static Lockscreen> = None;
    
}

mod sealed {
    pub trait Sealed {}
    impl<T: super::IsA<super::Lockscreen>> Sealed for T {}
}

pub trait LockscreenExt: IsA<Lockscreen> + sealed::Sealed + 'static {
    #[doc(alias = "phosh_lockscreen_get_page")]
    #[doc(alias = "get_page")]
    fn page(&self) -> LockscreenPage {
        unsafe {
            from_glib(ffi::phosh_lockscreen_get_page(self.as_ref().to_glib_none().0))
        }
    }

    #[doc(alias = "phosh_lockscreen_set_page")]
    fn set_page(&self, page: LockscreenPage) {
        unsafe {
            ffi::phosh_lockscreen_set_page(self.as_ref().to_glib_none().0, page.into_glib());
        }
    }

    fn carousel(&self) -> Option<libhandy::Carousel> {
        ObjectExt::property(self.as_ref(), "carousel")
    }

    #[doc(alias = "lockscreen-unlock")]
    fn connect_lockscreen_unlock<F: Fn(&Self) + 'static>(&self, f: F) -> SignalHandlerId {
        unsafe extern "C" fn lockscreen_unlock_trampoline<P: IsA<Lockscreen>, F: Fn(&P) + 'static>(this: *mut ffi::PhoshLockscreen, f: glib::ffi::gpointer) {
            let f: &F = &*(f as *const F);
            f(Lockscreen::from_glib_borrow(this).unsafe_cast_ref())
        }
        unsafe {
            let f: Box_<F> = Box_::new(f);
            connect_raw(self.as_ptr() as *mut _, b"lockscreen-unlock\0".as_ptr() as *const _,
                Some(std::mem::transmute::<_, unsafe extern "C" fn()>(lockscreen_unlock_trampoline::<Self, F> as *const ())), Box_::into_raw(f))
        }
    }

    #[doc(alias = "wakeup-output")]
    fn connect_wakeup_output<F: Fn(&Self) + 'static>(&self, f: F) -> SignalHandlerId {
        unsafe extern "C" fn wakeup_output_trampoline<P: IsA<Lockscreen>, F: Fn(&P) + 'static>(this: *mut ffi::PhoshLockscreen, f: glib::ffi::gpointer) {
            let f: &F = &*(f as *const F);
            f(Lockscreen::from_glib_borrow(this).unsafe_cast_ref())
        }
        unsafe {
            let f: Box_<F> = Box_::new(f);
            connect_raw(self.as_ptr() as *mut _, b"wakeup-output\0".as_ptr() as *const _,
                Some(std::mem::transmute::<_, unsafe extern "C" fn()>(wakeup_output_trampoline::<Self, F> as *const ())), Box_::into_raw(f))
        }
    }

    #[doc(alias = "carousel")]
    fn connect_carousel_notify<F: Fn(&Self) + 'static>(&self, f: F) -> SignalHandlerId {
        unsafe extern "C" fn notify_carousel_trampoline<P: IsA<Lockscreen>, F: Fn(&P) + 'static>(this: *mut ffi::PhoshLockscreen, _param_spec: glib::ffi::gpointer, f: glib::ffi::gpointer) {
            let f: &F = &*(f as *const F);
            f(Lockscreen::from_glib_borrow(this).unsafe_cast_ref())
        }
        unsafe {
            let f: Box_<F> = Box_::new(f);
            connect_raw(self.as_ptr() as *mut _, b"notify::carousel\0".as_ptr() as *const _,
                Some(std::mem::transmute::<_, unsafe extern "C" fn()>(notify_carousel_trampoline::<Self, F> as *const ())), Box_::into_raw(f))
        }
    }
}

impl<O: IsA<Lockscreen>> LockscreenExt for O {}