
use phrog::lockscreen::Lockscreen;
use gtk::glib::StaticType;
use std::ffi::{c_int, CString};
use libphosh::WallClock;
use libphosh::prelude::*;
use phrog::shell::Shell;

extern "C" {
    fn hdy_init();
    fn cui_init(v: c_int);
}

fn main() {
    gtk::gio::resources_register_include!("phrog.gresource")
        .expect("Failed to register resources.");

    gtk::init().unwrap();

    // This is necessary to ensure the Lockscreen type is actually registered. Otherwise it's done
    // lazily the first time it's instantiated, but we're currently hacking the type lookup in
    // phosh_lockscreen_new
    let _ = Lockscreen::static_type();

    let wall_clock = WallClock::new();
    wall_clock.set_default();

    unsafe {
        // let loglevel =
        //     CString::new(std::env::var("G_MESSAGES_DEBUG").unwrap_or("".to_string())).unwrap();
        // phosh_log_set_log_domains(loglevel.as_ptr());
        hdy_init();
        // cui_init(1);
    }

    let shell = Shell::new();
    shell.set_default();

    shell.connect_ready(|_| {
        println!("Shell is ready");
    });

    shell.set_locked(true);

    gtk::main();
}