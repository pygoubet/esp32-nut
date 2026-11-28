#pragma once
#include "core/notifications_settings_store.h"
class SmtpNotifier { public: static void sendAsync(const NotificationsSettings&, const String&, const String&); };