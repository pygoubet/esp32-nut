#include "core/notifications_settings_store.h"
#include <Preferences.h>
static constexpr const char* NVS_NS = "notifications";

NotificationsSettings NotificationsSettingsStore::load() {
    NotificationsSettings s;
    Preferences p;
    if (p.begin(NVS_NS,true)) {
        s.on_battery=p.getBool("ev_battery",true);
        s.back_online=p.getBool("ev_online",true);
        s.on_startup=p.getBool("ev_start",false);
        s.smtp_enabled=p.getBool("smtp_on",false);
        s.smtp_server=p.getString("smtp_host","");
        s.smtp_port=p.getUShort("smtp_port",465);
        s.smtp_tls=p.getBool("smtp_tls",true);
        s.smtp_username=p.getString("smtp_user","");
        s.smtp_password=p.getString("smtp_pass","");
        s.smtp_email_from=p.getString("smtp_from","");
        s.smtp_email_to=p.getString("smtp_to","");
        p.end();
    } return s; 
}

bool NotificationsSettingsStore::save(const NotificationsSettings& s) {
    Preferences p;
    if (!p.begin(NVS_NS,false)) return false;

    p.putBool("ev_battery",s.on_battery);
    p.putBool("ev_online",s.back_online);
    p.putBool("ev_start",s.on_startup);
    p.putBool("smtp_on",s.smtp_enabled);
    p.putString("smtp_host",s.smtp_server);
    p.putUShort("smtp_port",s.smtp_port);
    p.putBool("smtp_tls",s.smtp_tls);
    p.putString("smtp_user",s.smtp_username);
    p.putString("smtp_pass",s.smtp_password);
    p.putString("smtp_from",s.smtp_email_from);
    p.putString("smtp_to",s.smtp_email_to);
    p.end();
    return true;
}