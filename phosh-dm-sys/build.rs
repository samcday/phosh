use std::path::PathBuf;

mod build_version;

#[cfg(docsrs)]
fn main() {} // prevent linking libraries to avoid documentation failure

#[cfg(not(docsrs))]
fn main() {
    // This entire script is a giant ugly hack to build the in-tree (submoduled) phosh as a static
    // lib, and link it + all its dependent bits into the phrog binary.
    // Eventually phosh will be modified to output a `libphosh-dm.so` + pkg-config and then we
    // can go back to the default (autogenerated) build.rs from gtk-rs/gir (which automatically
    // links the shared lib via the systemd-dependencies crate).

    let phosh_src_path = std::path::PathBuf::from("../phosh").canonicalize().unwrap();
    let phosh_build_path = phosh_src_path.join("_build");

    // Rebuild if the static lib has changed, allow for quicker local rebuild runs:
    // meson compile -C phosh/_build && cargo run
    println!("cargo:rerun-if-changed={}", phosh_build_path.join("src/libphosh.a").to_str().unwrap());

    println!("cargo:rustc-link-lib=static=phosh-tool");
    println!("cargo:rustc-link-lib=static=phosh");
    println!("cargo:rustc-link-search=native={}", phosh_build_path.join("src").to_str().unwrap());
    meson::build(phosh_src_path.to_str().unwrap(), phosh_build_path.to_str().unwrap());

    println!("cargo:rustc-link-search=native={}", phosh_build_path.join("subprojects/libcall-ui/src").to_str().unwrap());
    println!("cargo:rustc-link-lib=static=call-ui");

    println!("cargo:rustc-link-search=native={}", phosh_build_path.join("subprojects/gvc").to_str().unwrap());
    println!("cargo:rustc-link-lib=static=gvc");

    println!("cargo:rustc-link-search=native={}", phosh_build_path.join("subprojects/gmobile/src").to_str().unwrap());
    println!("cargo:rustc-link-lib=static=gmobile");

    println!("cargo:rustc-link-lib=wayland-client");
    println!("cargo:rustc-link-lib=nm");
    println!("cargo:rustc-link-lib=secret-1");
    println!("cargo:rustc-link-lib=handy-1");
    println!("cargo:rustc-link-lib=pulse");
    println!("cargo:rustc-link-lib=pulse-mainloop-glib");
    println!("cargo:rustc-link-lib=fribidi");
    println!("cargo:rustc-link-lib=gudev-1.0");
    println!("cargo:rustc-link-lib=gmodule-2.0");
    println!("cargo:rustc-link-lib=systemd");
    println!("cargo:rustc-link-lib=polkit-agent-1");
    println!("cargo:rustc-link-lib=polkit-gobject-1");
    println!("cargo:rustc-link-lib=pam");
    println!("cargo:rustc-link-lib=callaudio-0.1");
    println!("cargo:rustc-link-lib=gcr-base-3");
    println!("cargo:rustc-link-lib=gcr-ui-3");
    println!("cargo:rustc-link-lib=feedback-0.0");
    println!("cargo:rustc-link-lib=gnome-desktop-3");
    println!("cargo:rustc-link-lib=soup-3.0");
    println!("cargo:rustc-link-lib=upower-glib");
    println!("cargo:rustc-link-lib=json-glib-1.0");
}
