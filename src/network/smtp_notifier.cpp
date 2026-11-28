#include "network/smtp_notifier.h"
#include <ESP_Mail_Client.h>
struct SmtpPayload { NotificationsSettings s; String subject, body; };
void SmtpNotifier::sendAsync(const NotificationsSettings& s,const String& subject,const String& body) {
    if (s.smtp_server.isEmpty() || s.smtp_email_from.isEmpty() || s.smtp_email_to.isEmpty())
        return;

    auto* p=new SmtpPayload{s,subject,body};
    if (xTaskCreate([](void* arg) {
        auto* q=static_cast<SmtpPayload*>(arg);
        SMTPSession smtp;
        Session_Config config;
        config.server.host_name=q->s.smtp_server.c_str();
        config.server.port=q->s.smtp_port;
        config.login.email=q->s.smtp_username.c_str();
        config.login.password=q->s.smtp_password.c_str();
        config.secure.startTLS=q->s.smtp_tls;
        SMTP_Message message;
        message.sender.name="ESP32-NUT";
        message.sender.email=q->s.smtp_email_from.c_str();
        message.subject=q->subject.c_str();
        message.addRecipient("Recipient",q->s.smtp_email_to.c_str());
        message.text.content=q->body.c_str();
        message.text.charSet="utf-8";
        smtp.connect(&config);
        MailClient.sendMail(&smtp,&message);
        smtp.closeSession();
        delete q;
        vTaskDelete(nullptr);
    },"smtp_send",8192,p,1,nullptr)!=pdPASS) delete p;
}