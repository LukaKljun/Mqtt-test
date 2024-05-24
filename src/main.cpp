#include <Arduino.h>
#include "WiFi.h"
#include <mqtt_supported_features.h>
#include <mqtt_client.h>
#include <esp_log.h>

#define TAG "MQTT"
esp_mqtt_client_config_t config;
esp_mqtt_client_handle_t client;

static void log_errors_only(const char *message, int error_code) {
    if (error_code != 0) {
        log_e("Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    log_d("Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    // Note: Multiple MQTT_EVENT_DATA could be fired for one message, if it is
    // longer than internal buffer. In that case only first event contains topic
    // pointer and length, other contain data only with current data length
    // and current data offset updating.

    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        log_i("MQTT_EVENT_CONNECTED");
        break;
    case MQTT_EVENT_DISCONNECTED:
        log_i("MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        log_i("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        log_i("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        log_i("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        log_i("MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        log_e("MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_errors_only("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_errors_only("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_errors_only("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            log_e("Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        log_i("Other event id:%d", event->event_id);
        break;
    }
}

void setup() {
    WiFi.begin("iLuka", "23456789");
    delay(6000);

    Serial.begin(115200);

    config.client_id = "Bananaman";
    config.host = "nether.mojvegovc.si";
    config.password = "PASSWORD";
    config.username = "USERNAME";
    config.port = 1883;
    config.lwt_msg = "Cleint298484 disconnected";
    config.lwt_topic = "banana";
    config.lwt_retain = false;
    config.lwt_msg_len = strlen(config.lwt_msg);

    client = esp_mqtt_client_init(&config);
    esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
    bool started = esp_mqtt_client_start(client) == ESP_OK;
    log_i("Client has %s started.", started ? "" : "not");
    log_i("Client connection to %s ", client != 0 ? "successfull." : "failed.");
    long a = esp_mqtt_client_publish(client, "banananjam", "#FF00FF", sizeof("#ff0333") - 1, 1, 1);
    log_i("Publish: %d %s", a, (a == -1) ? "failed." : "success.");
    esp_mqtt_client_subscribe(client, "banana", 2);
}

void loop() {
    delay(1000);
}
