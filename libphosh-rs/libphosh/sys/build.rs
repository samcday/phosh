// Generated by gir (https://github.com/gtk-rs/gir @ 5223ce91b97a)
// from ../.. (@ a6086de371d6+)
// from ../../gir-files (@ 6cd7b656acd6)
// DO NOT EDIT

#[cfg(not(docsrs))]
use std::process;

#[cfg(docsrs)]
fn main() {} // prevent linking libraries to avoid documentation failure

#[cfg(not(docsrs))]
fn main() {
    if let Err(s) = system_deps::Config::new().probe() {
        println!("cargo:warning={s}");
        process::exit(1);
    }
}
