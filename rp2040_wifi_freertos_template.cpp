/*
 * @brief Rack Inteligente, firmware responsável para gerar leituras de
 * temperatura, estado das portas, teclado e outros sensores do rack
 *
 * @author Carlos Delfino <consultoria@carlosdelfino.eti.br>
 * @version 0.1.0
 * @date 2025-11-16
 */
#include <pico/time.h>
#include <pico/stdio.h>
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"

#include <stdio.h>
#include <string.h>

#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"

#include "FreeRTOS.h"
#include "task.h"

#include "log_vt100.h"

#include "network_poll_task.h"

#define RACK_NETWORK_POLL_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define RACK_NETWORK_POLL_TASK_PRIORITY   1 
//#define WIFI_SSID     "ID do seu WiFi"
//#define WIFI_PASSWORD "Password do seu WiFi"
#define WIFI_TIMEOUT  10000

extern "C" {

// variáveis uteis a outras partes do código;
bool mqtt_connected;
mqtt_client_t *mqtt_client;


// Variáveis Locais
static ip_addr_t broker_ip;

// Protótipos de Funções
static void mqtt_connection_callback(mqtt_client_t *client, void *arg,
                                     mqtt_connection_status_t status);

static void dns_check_callback(const char *name, const ip_addr_t *ipaddr,
                               void *callback_arg);

static void inpub_cb(void *arg, const char *topic_in, u32_t tot_len);
static void indata_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);

// Função Principal
int main() {
  stdio_init_all();
  log_set_level(LOG_LEVEL_INFO);

  LOG_INFO("############################################");
  LOG_INFO("# RP2040 WiFi FreeRTOS Template            #");
  LOG_INFO("# Versão: 0.1.0                            #");
  LOG_INFO("# Autor: Carlos Delfino                    #");
  LOG_INFO("# E-Mail: consultoria@carlosdelfino.eti.br #");
  LOG_INFO("# Data: 2025-11-16                         #");
  LOG_INFO("############################################");

  LOG_INFO("Contagem regressiva... ____");
  for (int i = 200; i > 0; i--) {
    LOG_INFO("\r\x1b[4C%04d", i);
    sleep_ms(1);
  }

  // Inicializa Wi-Fi
  LOG_INFO("[Wi-Fi] Inicializando...");
  if (cyw43_arch_init()) {
    LOG_WARN("[Wi-Fi] Erro na inicialização do Wi-Fi");
    return -1;
  }
  cyw43_arch_enable_sta_mode();
  

  LOG_INFO("[Wi-Fi] Conectando...");
  if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                         CYW43_AUTH_WPA2_AES_PSK, WIFI_TIMEOUT)) {
    LOG_WARN("[Wi-Fi] Falha na conexão Wi-Fi");
    return -1;
  } else {
    LOG_INFO("[Wi-Fi] Conectado com sucesso!");
  }

  // Inicializa cliente MQTT
  LOG_INFO("[MQTT] Inicializando cliente MQTT...");
  mqtt_client = mqtt_client_new();

  // Resolve DNS do broker MQTT
  LOG_INFO("[DNS] Resolvendo broker MQTT...");
  err_t err =
      dns_gethostbyname(MQTT_BROKER, &broker_ip, dns_check_callback, NULL);
  if (err == ERR_OK) {
    dns_check_callback(MQTT_BROKER, &broker_ip, NULL);
  } else if (err == ERR_INPROGRESS) {
    LOG_INFO("[DNS] Resolvendo...");
    while (ip_addr_isany(&broker_ip)) sleep_ms(1000);
  } else {
    LOG_WARN("[DNS] Erro ao resolver DNS: %d", err);
    return -1;
  }

  LOG_INFO("[DNS] Resolvido: %s -> %s", MQTT_BROKER, ipaddr_ntoa(&broker_ip));

  if (mqtt_client == NULL) {
    LOG_WARN("[MQTT] Erro ao inicializar cliente MQTT");
    return -1;
  }

  LOG_INFO("[MQTT] Conectando ao broker MQTT...");
  mqtt_set_inpub_callback(mqtt_client, inpub_cb, indata_cb, NULL);

  // Cria tasks FreeRTOS para cada funcionalidade
  LOG_INFO("[FreeRTOS] Criando tasks...");

  xTaskCreate(vNetworkPollTask, "network_poll_task",
              RACK_NETWORK_POLL_TASK_STACK_SIZE,
              NULL, RACK_NETWORK_POLL_TASK_PRIORITY,
              NULL);
  
  // Inicia o scheduler do FreeRTOS (não retorna em operação normal)
  LOG_INFO("[FreeRTOS] Iniciando scheduler...");
  vTaskStartScheduler();

  // Se chegar aqui, houve falha ao iniciar o scheduler
  for (;;) {
    tight_loop_contents();
  }
}

static void inpub_cb(void *arg, const char *topic_in, u32_t tot_len) {
  LOG_INFO("[MQTT] Mensagem chegando no tópico: %s, tamanho: %lu", topic_in,
           (unsigned long)tot_len);
  char topic[50];
  snprintf(topic, sizeof(topic), "%s/%s/um_topico", MQTT_BASE_TOPIC,
           MQTT_RACK_NUMBER);
  if (strcmp(topic_in, topic) == 0) {
    //
  }
}

static void indata_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
  // data NÃO é terminado em '\0', então cuidado
  char payload[65];
  u16_t copy_len = (len < (sizeof(payload) - 1)) ? len : (sizeof(payload) - 1);
  memcpy(payload, data, copy_len);
  payload[copy_len] = '\0';
  LOG_INFO("[MQTT] Dados recebidos (len=%u): %s", len, payload);
  
  // processa os dados do tópico recem recebido
}


// Callback de conexão MQTT
static void mqtt_connection_callback(mqtt_client_t *client, void *arg,
                                     mqtt_connection_status_t status) {
  mqtt_connected = (status == MQTT_CONNECT_ACCEPTED);
}

// Callback de DNS
void dns_check_callback(const char *name, const ip_addr_t *ipaddr,
                        void *callback_arg) {

  if (ipaddr != NULL) {
    broker_ip = *ipaddr;
    LOG_INFO("[DNS] Broker IP: %s", ipaddr_ntoa(&broker_ip));

    LOG_INFO("[MQTT] Client ID: %s", MQTT_RACK_NUMBER);
    struct mqtt_connect_client_info_t ci = {
        MQTT_CLIENT_ID,
        MQTT_USERNAME,
        MQTT_PASSWORD,
        60,
        NULL,
        NULL,
        2,
        false
        // plus any extra fields in the correct order, if they exist
    };
    LOG_INFO("[MQTT] Conectando ao broker MQTT...");
    mqtt_client_connect(mqtt_client, &broker_ip, MQTT_PORT,
                        mqtt_connection_callback, NULL, &ci);
  }
}
}
