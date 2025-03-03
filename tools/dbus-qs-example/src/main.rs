use zbus::object_server::SignalEmitter;
use zbus::zvariant::{ObjectPath, OwnedObjectPath};
use zbus::Connection;

struct QuickSetting {
    active: bool,
    counter: i32,
    status_path: OwnedObjectPath,
}

#[zbus::interface(name = "mobi.phosh.shell.QuickSetting")]
impl QuickSetting {
    async fn press(&mut self, #[zbus(signal_emitter)] emitter: SignalEmitter<'_>) {
        self.counter += 1;
        self.label_changed(&emitter).await.unwrap();
        self.icon_changed(&emitter).await.unwrap();
    }

    async fn long_press(&mut self, #[zbus(signal_emitter)] emitter: SignalEmitter<'_>) {
        if self.counter <= 0 {
            self.counter -= 1;
        } else {
            self.active = !self.active;
        }
        self.label_changed(&emitter).await.unwrap();
        self.active_changed(&emitter).await.unwrap();
        self.icon_changed(&emitter).await.unwrap();
    }

    #[zbus(property)]
    fn status(&self) -> ObjectPath {
        self.status_path.clone().into()
    }

    #[zbus(property)]
    async fn label(&self) -> String {
        format!("Count: {}", self.counter)
    }

    #[zbus(property)]
    async fn icon(&self) -> &str {
        if self.active {
            "image-loading-symbolic"
        } else if self.counter > 0 {
            "emblem-favorite-symbolic"
        } else if self.counter < 0 {
            "dialog-question-symbolic"
        } else {
            "emoji-people-symbolic"
        }
    }

    #[zbus(property)]
    async fn name(&self) -> &str {
        "DBus Quick Setting demo"
    }

    #[zbus(property)]
    async fn description(&self) -> &str {
        "A demonstration of making a QuickSetting available entirely via DBus. Look ma no .so!"
    }

    #[zbus(property)]
    async fn active(&self) -> bool {
        self.active
    }

    #[zbus(property)]
    async fn set_active(&mut self, active: bool) {
        self.active = active;
    }
}

struct QuickSettingStatus {
    header_activatable: bool,
    footer_activatable: bool,
    footer_count: u32,
}

#[zbus::interface(name = "mobi.phosh.shell.qs.Status")]
impl QuickSettingStatus {
    #[zbus(property)]
    async fn title(&self) -> &str {
        "DBus Quick Setting Demo"
    }

    #[zbus(property, name = "Type")]
    async fn type_(&self) -> &str {
        "placeholder"
    }

    #[zbus(property)]
    async fn header_text(&self) -> &str {
        ""
    }

    #[zbus(property)]
    async fn header_icon(&self) -> &str {
        "face-angel-symbolic"
    }

    #[zbus(property)]
    async fn header_activatable(&self) -> bool {
        self.header_activatable
    }

    #[zbus(property)]
    async fn footer_text(&self) -> String {
        if self.footer_activatable {
            if self.footer_count == 0 {
                "Click me!".into()
            } else {
                format!("Clicked {} times", self.footer_count)
            }
        } else {
            "I'm a label".into()
        }
    }
    #[zbus(property)]
    async fn footer_icon(&self) -> &str {
        if self.footer_activatable {
            "input-mouse-symbolic"
        } else {
            "emblem-system-symbolic"
        }
    }

    #[zbus(property)]
    async fn footer_activatable(&self) -> bool {
        self.footer_activatable
    }

    async fn activate_footer(&mut self, #[zbus(signal_emitter)] emitter: SignalEmitter<'_>) {
        self.footer_count += 1;
        self.footer_text_changed(&emitter).await.unwrap();
    }

    async fn activate_header(&mut self, #[zbus(signal_emitter)] emitter: SignalEmitter<'_>) {
        self.footer_activatable = !self.footer_activatable;
        self.footer_count = 0;
        self.footer_activatable_changed(&emitter).await.unwrap();
        self.footer_icon_changed(&emitter).await.unwrap();
        self.footer_text_changed(&emitter).await.unwrap();
    }
}

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    let connection = Connection::session().await?;

    let path = ObjectPath::from_static_str_unchecked("/com/samcday/QSDemo");

    let qs = QuickSetting {
        active: false,
        counter: 0,
        status_path: path.clone().into(),
    };

    connection.object_server().at(&path, qs).await?;
    connection.object_server().at(&path, QuickSettingStatus {
        footer_activatable: true,
        header_activatable: true,
        footer_count: 0,
    }).await?;
    connection.request_name("com.samcday.QSDemo").await?;

    let qs_ref = connection.object_server().interface::<_, QuickSetting>(&path).await?;

    let mut ticker = tokio::time::interval(std::time::Duration::from_millis(500));
    loop {
        ticker.tick().await;
        let mut qs = qs_ref.get_mut().await;
        if qs.active {
            if (qs.counter > 0) {
                qs.counter -= 1;
            }
            if (qs.counter == 0) {
                qs.active = false;
                qs.counter = 0;
                qs.icon_changed(qs_ref.signal_emitter()).await?;
                qs.active_changed(qs_ref.signal_emitter()).await?;
            }
            qs.label_changed(qs_ref.signal_emitter()).await?;
        }
    }
}
