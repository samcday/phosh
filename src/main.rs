mod keypad_shuffle;
mod lockscreen;
mod session_object;
mod sessions;
mod shell;
mod user_session_page;
mod users;

use std::io::{BufRead, BufReader, Read};
use std::process::Stdio;
use crate::shell::Shell;
use clap::Parser;
use gtk::{Application, gdk, gio};
use gtk::glib::{g_info, StaticType};
use libphosh::prelude::*;
use libphosh::WallClock;

pub const APP_ID: &str = "com.samcday.phrog";

const PHOC_RUNNING_PREFIX: &'static str = "Running compositor on wayland display '";

extern "C" {
    fn hdy_init();
    fn cui_init(v: core::ffi::c_int);
}

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    #[arg(short, long, help = "Launch nested phoc compositor if necessary")]
    phoc: bool,
}

fn main() {
    gio::resources_register_include!("phrog.gresource")
        .expect("Failed to register resources.");

    let args = Args::parse();

    // TODO: check XDG_RUNTIME_DIR here? Angry if not set? Default?

    let display = if args.phoc {
        // launch nested phoc, ensure the GDK in this process uses it.
        let mut phoc = std::process::Command::new("phoc")
            .stdout(Stdio::piped())
            .stdin(Stdio::null())
            .stderr(Stdio::null())
            .spawn()
            .unwrap();

        // Wait for startup message.
        let mut display = None;
        for line in BufReader::new(phoc.stdout.as_mut().unwrap()).lines() {
            let line = line.unwrap();
            if line.starts_with(PHOC_RUNNING_PREFIX) {
                display = Some(line.strip_prefix(PHOC_RUNNING_PREFIX).unwrap().strip_suffix("'").unwrap().to_string());
                break;
            }
        }
        let display = display.unwrap();
        g_info!("phrog", "phoc running '{}'", display);
        std::env::set_var("WAYLAND_DISPLAY", &display);

        ctrlc::set_handler(move || {
            phoc.kill().unwrap();
            phoc.wait().unwrap();
        }).unwrap();

        gdk::set_allowed_backends("wayland");
        gdk::init();
        gdk::Display::open(&display)
    } else {
        gdk::set_allowed_backends("wayland");
        gdk::init();
        gdk::Display::default()
    };

    if display.is_none() {
        panic!("failed GDK init");
    }

    gtk::init().unwrap();
    let _app = Application::builder().application_id(APP_ID).build();

    let wall_clock = WallClock::new();
    wall_clock.set_default();

    unsafe {
        // let loglevel =
        //     CString::new(std::env::var("G_MESSAGES_DEBUG").unwrap_or("".to_string())).unwrap();
        // phosh_log_set_log_domains(loglevel.as_ptr());
        hdy_init();
        cui_init(1);
    }

    let shell = Shell::new();
    shell.set_default();

    shell.connect_ready(|_| {
        println!("Shell is ready");
    });

    shell.set_locked(true);

    gtk::main();
}
