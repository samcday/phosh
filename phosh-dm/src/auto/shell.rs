// This file was generated by gir (https://github.com/gtk-rs/gir)
// from ..
// from ../gir-files
// DO NOT EDIT

use glib::{prelude::*,signal::{connect_raw, SignalHandlerId},translate::*};
use std::{boxed::Box as Box_};

glib::wrapper! {
    #[doc(alias = "PhoshShell")]
    pub struct Shell(Object<ffi::PhoshShell, ffi::PhoshShellClass>);

    match fn {
        type_ => || ffi::phosh_shell_get_type(),
    }
}

impl Shell {
        pub const NONE: Option<&'static Shell> = None;
    

    #[doc(alias = "phosh_shell_get_default")]
    #[doc(alias = "get_default")]
    #[allow(clippy::should_implement_trait)]    pub fn default() -> Option<Shell> {
        assert_initialized_main_thread!();
        unsafe {
            from_glib_none(ffi::phosh_shell_get_default())
        }
    }
}

mod sealed {
    pub trait Sealed {}
    impl<T: super::IsA<super::Shell>> Sealed for T {}
}

pub trait ShellExt: IsA<Shell> + sealed::Sealed + 'static {
    #[doc(alias = "phosh_shell_get_locked")]
    #[doc(alias = "get_locked")]
    fn is_locked(&self) -> bool {
        unsafe {
            from_glib(ffi::phosh_shell_get_locked(self.as_ref().to_glib_none().0))
        }
    }

    #[doc(alias = "phosh_shell_get_lockscreen_type")]
    #[doc(alias = "get_lockscreen_type")]
    fn lockscreen_type(&self) -> glib::types::Type {
        unsafe {
            from_glib(ffi::phosh_shell_get_lockscreen_type(self.as_ref().to_glib_none().0))
        }
    }

    #[doc(alias = "phosh_shell_set_default")]
    fn set_default(&self) {
        unsafe {
            ffi::phosh_shell_set_default(self.as_ref().to_glib_none().0);
        }
    }

    #[doc(alias = "phosh_shell_set_locked")]
    fn set_locked(&self, locked: bool) {
        unsafe {
            ffi::phosh_shell_set_locked(self.as_ref().to_glib_none().0, locked.into_glib());
        }
    }

    fn is_docked(&self) -> bool {
        ObjectExt::property(self.as_ref(), "docked")
    }

    fn set_docked(&self, docked: bool) {
        ObjectExt::set_property(self.as_ref(),"docked", docked)
    }

    #[doc(alias = "ready")]
    fn connect_ready<F: Fn(&Self) + 'static>(&self, f: F) -> SignalHandlerId {
        unsafe extern "C" fn ready_trampoline<P: IsA<Shell>, F: Fn(&P) + 'static>(this: *mut ffi::PhoshShell, f: glib::ffi::gpointer) {
            let f: &F = &*(f as *const F);
            f(Shell::from_glib_borrow(this).unsafe_cast_ref())
        }
        unsafe {
            let f: Box_<F> = Box_::new(f);
            connect_raw(self.as_ptr() as *mut _, b"ready\0".as_ptr() as *const _,
                Some(std::mem::transmute::<_, unsafe extern "C" fn()>(ready_trampoline::<Self, F> as *const ())), Box_::into_raw(f))
        }
    }

    #[doc(alias = "docked")]
    fn connect_docked_notify<F: Fn(&Self) + 'static>(&self, f: F) -> SignalHandlerId {
        unsafe extern "C" fn notify_docked_trampoline<P: IsA<Shell>, F: Fn(&P) + 'static>(this: *mut ffi::PhoshShell, _param_spec: glib::ffi::gpointer, f: glib::ffi::gpointer) {
            let f: &F = &*(f as *const F);
            f(Shell::from_glib_borrow(this).unsafe_cast_ref())
        }
        unsafe {
            let f: Box_<F> = Box_::new(f);
            connect_raw(self.as_ptr() as *mut _, b"notify::docked\0".as_ptr() as *const _,
                Some(std::mem::transmute::<_, unsafe extern "C" fn()>(notify_docked_trampoline::<Self, F> as *const ())), Box_::into_raw(f))
        }
    }

    #[doc(alias = "locked")]
    fn connect_locked_notify<F: Fn(&Self) + 'static>(&self, f: F) -> SignalHandlerId {
        unsafe extern "C" fn notify_locked_trampoline<P: IsA<Shell>, F: Fn(&P) + 'static>(this: *mut ffi::PhoshShell, _param_spec: glib::ffi::gpointer, f: glib::ffi::gpointer) {
            let f: &F = &*(f as *const F);
            f(Shell::from_glib_borrow(this).unsafe_cast_ref())
        }
        unsafe {
            let f: Box_<F> = Box_::new(f);
            connect_raw(self.as_ptr() as *mut _, b"notify::locked\0".as_ptr() as *const _,
                Some(std::mem::transmute::<_, unsafe extern "C" fn()>(notify_locked_trampoline::<Self, F> as *const ())), Box_::into_raw(f))
        }
    }
}

impl<O: IsA<Shell>> ShellExt for O {}
