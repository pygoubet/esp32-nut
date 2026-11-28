#ifndef NETWORK_NOTIFICATIONS_NOTIFIER_H
#define NETWORK_NOTIFICATIONS_NOTIFIER_H

#include <Arduino.h>
#include "core/notifications_settings_store.h"

class NotificationsnNotifier {
public:
    static void sendAsync(const NotificationsSettings& settings, const String& subject, const String& body);
};

#endif // NETWORK_NOTIFICATIONS_NOTIFIER_H