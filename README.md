# Embarcatech-U7T1

## links úteis:

[Relatório](https://drive.google.com/file/d/1GtpYH8RU-0lxia5Zwx7Q-luvL2j1X5Uh/view?usp=sharing)
[Video](https://drive.google.com/file/d/1UaTDoonUdeKe41Mfk1ap8MS4KUDjbxsi/view?usp=sharing)

Este projeto tem o objetivo de monitorar o nível de dois tanques baseado em leitura de celulas de cargas, capazes de indicar o volume de material armazenado, independente do tipo de produto presente no tanque monitorado.

Além de monitorar os níveis para exibição no display, o sistema conta com funcionalidade de alarmes, permitindo ao usuário indicar os níveis mínimos desejados para que o sistema emita alarmes caso os níveis fiquem abaixo dos valores configurados.

## Funcionalidades das Teclas 🕹️

| Tecla   | Funcionalidade                                            |
| ------- | --------------------------------------------------------- |
| Botão A | Desliga sinal sonoro ou atualiza o setpointo pro tanque A |
| Botão B | Desliga sinal sonoro ou atualiza o setpointo pro tanque B |
| JOY Y   | Simula o nível no tanque A                                |
| JOY X   | Simula o nível no tanque B                                |

## Hardware 🛠️

- Microcontrolador RP2040 (Raspberry Pi Pico).
- Botões tipo Switch.
- Display i2c 128 x 64 pixels
- Buzzer para alarme sonoro
- Matriz de leds NeoPixel 5x5
- Células de carga com conversor analógico digital HX711 simuladas por Joystick analógico 2 eixos

## Software 💻

- **SDK do Raspberry Pi Pico:** O SDK (Software Development Kit) do Pico, que inclui as bibliotecas e ferramentas necessárias para desenvolver e compilar o código. [Instruções de instalação](https://www.raspberrypi.com/documentation/pico/getting-started/)
- **CMake:** Um sistema de construção multiplataforma usado para gerar os arquivos de construção do projeto.
- **Compilador C/C++:** Um compilador C/C++ como o GCC (GNU Compiler Collection).
- **Git:** (Opcional) Para clonar o repositório do projeto.

### O código está dividido em vários arquivos para melhor organização:

- **`U7T1.C`**: Código com a função de loop principal: lê os sensores analógicos, faz a conversão de valores e gera os alarmes
- **`display/ssd1306.c/.h`**: Comunica com o display via i2c.
- **`neopixel/neopixel.c/.h`**:Controla a matriz de leds para sinal visual de alarmes.
- **`CMakeLists.txt`:** Define a estrutura do projeto para o CMake.

## Como Compilar e Executar ⚙️

1. **Instale o SDK do Raspberry Pi Pico:** Siga as instruções no site oficial do Raspberry Pi.
2. **Clone este repositório:** `git clone git@github.com:klebersm/embarcatech-U7T1.git`
3. **Navegue até o diretório do projeto:** `cd Embarcatech-U7T1`
4. **Compile o projeto:** `cmake -B build && cmake --build build`
5. **Copie para o Pico:** Copie o arquivo `U7T1.uf2` da pasta `build` (gerada após a compilação) para o Raspberry Pi Pico. O programa iniciará automaticamente.

## Tratamento dos botões - Interrupções e Alarmes ⏱️

Os botões A e B foram inicializados como entradas em PULL UP e são monitorados por interrupções com uma função para o tratamento dessas interrupções. À medida que ocorre uma interrupção, a função de tratamento verifica qual o botão gerou o evento e qual o tipo de evento. Se o evento foi de FALL EDGE (borda de descida), significa que o botão foi pressionado e um alarme é gerado para ser disparado em 75ms (Tempo configurado para debounce). Se qualquer outro evento acontecer no mesmo botão, esse alarme é cancelado. Sendo assim, o alarme só é realmente disparado se o botão for pressionado e ficar 75ms sem sofrer qualquer alteração.

## 🔗 Link do Vídeo de Funcionamento:

## 📞 Contato

- 👤 **Autor**: Kleber Marçal

- 📧 **E-mail**:kleber.sm@gmail.com

---
