# RP2040 WiFi FreeRTOS Template

[![Visitors](https://komarev.com/ghpvc/?username=ArvoreDosSaberes&repo=rp2040_wifi_freertos_template&color=blue&style=flat)](https://github.com/ArvoreDosSaberes/rp2040_wifi_freertos_template)
[![Build](https://img.shields.io/github/actions/workflow/status/ArvoreDosSaberes/rp2040_wifi_freertos_template/ci.yml?branch=main)](https://github.com/ArvoreDosSaberes/rp2040_wifi_freertos_template/actions)
[![Issues](https://img.shields.io/github/issues/ArvoreDosSaberes/rp2040_wifi_freertos_template)](https://github.com/ArvoreDosSaberes/rp2040_wifi_freertos_template/issues)
[![Stars](https://img.shields.io/github/stars/ArvoreDosSaberes/rp2040_wifi_freertos_template)](https://github.com/ArvoreDosSaberes/rp2040_wifi_freertos_template/stargazers)
[![Forks](https://img.shields.io/github/forks/ArvoreDosSaberes/rp2040_wifi_freertos_template)](https://github.com/ArvoreDosSaberes/rp2040_wifi_freertos_template/network/members)
[![FreeRTOS](https://img.shields.io/badge/RTOS-FreeRTOS-9cf.svg)](https://www.freertos.org/)
[![Language](https://img.shields.io/badge/Language-C%2FC%2B%2B-brightgreen.svg)]()
[![License: CC BY 4.0](https://img.shields.io/badge/license-CC%20BY%204.0-blue.svg)](https://creativecommons.org/licenses/by/4.0/)

Template de projeto para o microcontrolador **RP2040** utilizando:

- FreeRTOS
- WiFi
- MQTT

Voltado para aplicações IoT embarcadas, com estrutura organizada em *tasks* e facilidades para criação de novas tarefas que utilizam conectividade.

---

## Autor

- Nome: Carlos Delfino  
- E-mail: consultoria@carlosdelfino.eti.br

---

## Licença

Este projeto está licenciado sob os termos da  
**Creative Commons Attribution 4.0 International (CC BY 4.0)**.

Você é livre para:

- Compartilhar — copiar e redistribuir o material em qualquer suporte ou formato  
- Adaptar — remixar, transformar, e criar a partir do material para qualquer fim, mesmo que comercial

Desde que respeite a Atribuição:

- Deve dar o devido crédito a Carlos Delfino
- Incluir um link para a licença
- Indicar se foram feitas modificações

Mais detalhes em: https://creativecommons.org/licenses/by/4.0/

---

## Estrutura do Projeto

(Adapte esta seção conforme a organização exata do seu código.)

- `CMakeLists.txt`  
  Configuração principal do projeto (nome do executável, fontes, includes etc.).

- `rp2040_wifi_freertos_template.cpp`  
  Ponto de entrada, inicialização básica de hardware e de FreeRTOS.

- `tasks/`  
  Diretório para as tasks do FreeRTOS, por exemplo:
  - `network_poll_task.c` — task responsável por gerenciar o estado de rede (WiFi/MQTT), reconexão, etc.
  - Outras tasks de aplicação (sensores, atuadores, lógica de negócio).

- `pico_sdk_import.cmake`  
  Integração com o Raspberry Pi Pico SDK.

---

## Como usar o projeto

### Pré-requisitos

- Toolchain C/C++ para RP2040 (ex.: gcc-arm-none-eabi)  
- CMake  
- Ninja ou make  
- Raspberry Pi Pico SDK configurado (`PICO_SDK_PATH`)  
- Git

### Clonar o repositório

```bash
git clone https://github.com/user/repo.git
cd repo
```

(Substitua `user/repo` pelo caminho correto do seu GitHub.)

### Gerar build

```bash
mkdir build
cd build
cmake .. -G "Ninja"
ninja
```

O binário/UF2 será gerado em `build/` (por exemplo `rp2040_wifi_freertos_template.uf2`).

### Gravar no RP2040

1. Conecte o RP2040 ao PC segurando o botão BOOTSEL.  
2. Aguarde montar como unidade USB (`RPI-RP2`).  
3. Copie o arquivo `.uf2` gerado em `build/` para essa unidade.  
4. A placa irá reinicializar rodando o firmware deste projeto.

---

## Configurando WiFi e MQTT

Ajuste os nomes das variáveis de acordo com o seu código real.

### WiFi

Defina SSID e senha em um arquivo de configuração (por exemplo `config_wifi.h`):

```c
#define WIFI_SSID     "SuaRede"
#define WIFI_PASSWORD "SuaSenha"
```

Na inicialização de rede (por exemplo em `network_poll_task.c`):

- Inicialize o driver de WiFi do RP2040.  
- Configure modo estação (STA).  
- Conecte usando `WIFI_SSID` e `WIFI_PASSWORD`.  
- Aguarde obter IP (DHCP).  
- Sinalize para outras tasks que a rede está pronta (EventGroup, Queue, etc.).

### MQTT

Defina as configurações básicas em um header (por exemplo `config_mqtt.h`):

```c
#define MQTT_BROKER_HOST "broker.hivemq.com"
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID   "rp2040_client"
#define MQTT_USERNAME    NULL
#define MQTT_PASSWORD    NULL
#define MQTT_TOPIC_SUB   "meu/topico/sub"
#define MQTT_TOPIC_PUB   "meu/topico/pub"
```

Na task MQTT:

- Aguarde a sinalização de que a rede/WiFi está pronta.  
- Crie e configure o cliente MQTT.  
- Conecte ao broker.  
- Faça subscribe nos tópicos necessários.  
- Mantenha um loop de processamento (recebimento e publicação de mensagens).

---

## Criando novas tasks FreeRTOS para WiFi e MQTT

### Estrutura básica de uma task

Uma task típica em FreeRTOS:

```c
void vMinhaTask(void *pvParameters)
{
    for (;;)
    {
        // Lógica da task
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

Criação da task (em `main` ou função de init):

```c
xTaskCreate(
    vMinhaTask,     // função
    "MinhaTask",   // nome
    1024,           // stack
    NULL,           // parâmetro
    1,              // prioridade
    NULL            // handle
);
```

### Task que depende de WiFi

Use um EventGroup (ou outra sinalização) para só iniciar o trabalho após a rede estar pronta, por exemplo:

- `network_poll_task.c` conecta ao WiFi e, quando tiver IP, seta um bit `NETWORK_READY_BIT` em `xNetworkEventGroup`.  
- Sua task WiFi aguarda esse bit antes de rodar a lógica que usa rede.

### Task para publicar via MQTT

Organize o código MQTT em uma task central que mantém a conexão e expõe filas (`QueueHandle_t`) para envio de mensagens.  
Outras tasks de aplicação enviam estruturas contendo `topic` e `payload` para essa fila, e a task MQTT realiza o `publish`.

### Boas práticas

- Evite busy-wait (`while(1)` sem delay). Use `vTaskDelay` ou primitivas de sincronização.  
- Separe responsabilidades: uma task para rede/WiFi, outra para MQTT, e outras para lógica de aplicação.  
- Defina prioridades coerentes (rede/MQTT podem ter prioridade levemente maior se forem críticos).  
- Proteja recursos compartilhados com mutex ou semáforos quando necessário.

---

## Contribuições

Sinta-se à vontade para abrir issues e pull requests no GitHub com:

- Correções de bugs  
- Melhorias na documentação  
- Exemplos adicionais de tasks WiFi/MQTT  
- Adaptações para outros módulos de WiFi ou brokers MQTT
