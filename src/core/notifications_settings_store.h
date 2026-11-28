#ifndef CORE_NOTIFICATIONS_SETTINGS_STORE_H
#define CORE_NOTIFICATIONS_SETTINGS_STORE_H

#include <Arduino.h>

struct NotificationsSettings {
    bool on_battery = true;
    bool back_online = true;
    bool on_startup = false;
    bool smtp_enabled = false;
    String smtp_server = "";
    uint16_t smtp_port = 465;
    bool smtp_tls = true;
    String smtp_username = "";
    String smtp_password = "";
    String smtp_email_from = "";
    String smtp_email_to = "";
};

class NotificationsSettingsStore {
public:
    NotificationsSettings load();
    bool save(const NotificationsSettings& settings);
};

#endif // CORE_NOTIFICATIONS_SETTINGS_STORE_H