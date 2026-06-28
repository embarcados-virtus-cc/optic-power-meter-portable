# Optic Power Meter — Portable

<div align="center">

**Um medidor de potência óptica portátil e open-source para transceptores SFP/SFP+**  
**An open-source portable optical power meter for SFP/SFP+ transceivers**

`Raspberry Pi Pico` · `SFF-8472` · `I²C` · `OLED SSD1306` · `C / Pico SDK`

</div>

---

## Português

### O que é este projeto?

Este dispositivo embarcado se conecta diretamente a um transceptor SFP/SFP+ e exibe, em um display OLED portátil, todos os seus dados internos — sem computador, sem software proprietário, sem equipamento industrial.

**Dados acessíveis:**

| Dado | Unidade | Fonte |
|---|---|---|
| Potência óptica recebida (Rx) | dBm / µW | A2h |
| Potência óptica transmitida (Tx) | dBm / µW | A2h |
| Temperatura interna | °C | A2h |
| Tensão de alimentação | V | A2h |
| Corrente de bias do laser | mA | A2h |
| Nome e P/N do fabricante | — | A0h |
| Comprimento de onda | nm | A0h |
| Alarmes e avisos ativos | flags | A2h |

---

### Por que este projeto existe?

Transceptores SFP expõem, via I²C, um mapa de memória padronizado (SFF-8472) com dados ricos de identificação e diagnóstico. O problema: **não há soluções embarcadas acessíveis** para consultá-los.

```
Opções existentes hoje:
┌─────────────────────────────────────────┐
│  Equipamento industrial de teste        │  Alto custo, não portátil
│  Software em computador                 │  Requer cabo e driver
│  Leitura manual de datasheet            │  Impraticável em campo
└─────────────────────────────────────────┘

Este projeto:
┌─────────────────────────────────────────┐
│  Raspberry Pi Pico + breakout SFP       │  < R$ 100 em componentes
│  Bateria + OLED portátil                │  Leitura em campo
│  Código aberto                          │  Extensível para QSFP, CMIS
└─────────────────────────────────────────┘
```

---

### Conceitos fundamentais

#### O que é um transceptor SFP?

SFP (Small Form-Factor Pluggable) é um módulo compacto que converte sinais elétricos em ópticos (e vice-versa). É o componente que você insere em switches, roteadores, OLTs e ONUs para conectar fibras ópticas.

```
          REDE ÓPTICA
              │
    ┌─────────┴─────────┐
    │  Fibra óptica LC  │
    └─────────┬─────────┘
              │  luz (1310nm / 1550nm)
    ┌─────────▼─────────┐
    │    Módulo SFP     │  ← Este projeto lê daqui
    │  ┌─────────────┐  │
    │  │  Fotodiodo  │  │  Rx: converte luz → corrente
    │  │  Laser VCSEL│  │  Tx: converte corrente → luz
    │  │  MCU + EEPROM│ │  I²C: expõe dados internos
    │  └─────────────┘  │
    └─────────┬─────────┘
              │  sinal elétrico SFI/SFF
    ┌─────────▼─────────┐
    │  Switch / Roteador│
    └───────────────────┘
```

#### O protocolo SFF-8472

A especificação SFF-8472 define como o módulo SFP organiza seus dados em memória e os expõe via I²C. Há dois endereços:

```
Barramento I²C
      │
      ├──► 0x50 (A0h) — Serial ID / Identificação
      │         │
      │         ├── [bytes  0–19]  Tipo, conector, encoding, velocidade
      │         ├── [bytes 20–35]  Vendor name  (ex: "FINISAR CORP.")
      │         ├── [bytes 40–55]  Vendor P/N   (ex: "FTLX8571D3BCL")
      │         ├── [bytes 60–61]  Wavelength   (ex: 850 nm)
      │         ├── [byte  63]     CC_BASE checksum
      │         ├── [bytes 64–94]  Extended ID
      │         │       └── [byte 92]  Diag. Monitoring Type ← habilita DMI
      │         └── [byte  95]     CC_EXT checksum
      │
      └──► 0x51 (A2h) — Diagnostics / DMI
                │
                ├── [bytes  0–55]  Limiares de alarme e aviso
                │       ├── Temp High/Low Alarm & Warning
                │       ├── VCC  High/Low Alarm & Warning
                │       ├── TX Bias High/Low Alarm & Warning
                │       ├── TX Power High/Low Alarm & Warning
                │       └── RX Power High/Low Alarm & Warning
                │
                ├── [bytes 56–91]  Medições em tempo real
                │       ├── [56–57]  Temperatura (°C)
                │       ├── [58–59]  VCC (V × 10000)
                │       ├── [60–61]  TX Bias (mA × 500)
                │       ├── [62–63]  TX Power (µW × 10000)
                │       └── [64–65]  RX Power (µW × 10000)
                │
                ├── [byte 110]    Data_Not_Ready flag ← verificado antes de ler
                └── [bytes 96–127] Controle e páginas opcionais
```

#### Como os dados são convertidos para unidades físicas?

Os valores brutos de 16 bits são lidos e convertidos por fórmulas da spec:

```c
// Temperatura: signed 16-bit, complemento de dois
// LSB = 1/256 °C
float temp_C = (int16_t)raw_temp / 256.0f;

// VCC: unsigned 16-bit
// LSB = 100 µV  →  divide por 10000 para volts
float vcc_V = raw_vcc / 10000.0f;

// TX Bias: unsigned 16-bit
// LSB = 2 µA  →  divide por 500 para mA
float bias_mA = raw_bias / 500.0f;

// Potência óptica (RX/TX): unsigned 16-bit
// LSB = 0.1 µW  →  divide por 10000 para mW
// Para dBm: 10 * log10(power_mW)
float power_uW = raw_power / 10.0f;
float power_dBm = 10.0f * log10f(power_uW / 1000.0f);
```

#### O que é DMI?

DMI (Digital Monitoring Interface) é o recurso que permite ao próprio transceptor expor medições calibradas. Antes de ler A2h, o firmware verifica se DMI está habilitado no byte 92 de A0h:

```c
// sfp.c — verificação real (corrigida em relação à versão original)
bool check_sfp_a2h_exists(sfp_t *sfp) {
    // Bit 6 do byte 92 = "Digital diagnostics implemented"
    return (sfp->a0e.diag_monitoring_type >> 6) & 0x01;
}
```

Se o bit não estiver setado, o módulo não suporta DMI e a leitura de A2h retornará dados inválidos.

---

### Arquitetura do firmware

O firmware é organizado em três camadas independentes:

```
┌──────────────────────────────────────────────────────────────┐
│                        APLICAÇÃO                              │
│                                                               │
│   menu/menu.c          main.c        Grafico_Experimental.py  │
│   ─────────────        ──────        ─────────────────────    │
│   7 telas OLED:        Loop 50 ms:   Análise offline:         │
│   • Menu principal     init I²C      Plotagem de dados        │
│   • Diagnóstico        init OLED     exportados via USB       │
│   • Alarmes            init joystick                          │
│   • Status             ┌──────────┐                           │
│   • Info. módulo       │joystick  │                           │
│   • Monitoramento      │SFP update│                           │
│   • Config.            │render    │                           │
│                        └──────────┘                           │
├──────────────────────────────────────────────────────────────┤
│                        PROTOCOLO                              │
│                                                               │
│   sfp_8472/                    I2C/            ssd1306/       │
│   ──────────                   ────            ────────       │
│   a0h.c + a0h.h                i2c.c           ssd1306.c      │
│   → Parser A0h (128 bytes)     i2c.h           ssd1306.h      │
│   → Campos de ID e ext.        Abstração HAL   Driver OLED    │
│                                                               │
│   a2h.c + a2h.h                joystick/                      │
│   → Parser A2h (128 bytes)     ─────────                      │
│   → Conversão para float       JoystickPi.c                   │
│   → Validação Data_Not_Ready   Driver ADC 5 vias              │
│                                                               │
│   sfp.c + sfp.h                                               │
│   → sfp_init() / sfp_update()                                 │
│   → Orquestra leitura A0+A2                                   │
│   → Valida CC_BASE e CC_EXT                                   │
├──────────────────────────────────────────────────────────────┤
│                   HARDWARE — RP2040 / Pico SDK                │
│                                                               │
│   hardware_i2c      hardware_adc                              │
│   ────────────      ────────────                              │
│   Periférico I²C    Joystick ADC                              │
│   400 kHz           12-bit, 5 ch.                             │
└──────────────────────────────────────────────────────────────┘
```

**Fluxo de execução:**

```
main()
  │
  ├─ ssd1306_Init()          → inicializa display OLED via I²C
  ├─ joystickPi_init()       → configura ADC para joystick
  ├─ sfp_i2c_init(100 kHz)   → configura barramento I²C ← DEVE ser antes de ler SFP
  ├─ sleep_ms(2000)          → aguarda estabilização do módulo SFP
  ├─ init_sfp_data()         → lê A0h, valida checksums, verifica DMI
  │
  └─ while(true) [50 ms]
       ├─ process_joystick_input()   → detecta direção/clique
       ├─ update_system_data()       → lê A0h + A2h via I²C
       └─ render_current_screen()    → desenha tela no OLED
```

---

### Hardware

```
┌─────────────────────────────────────────────────────────────┐
│                    Diagrama de conexões                      │
│                                                              │
│   ┌──────────────┐      I²C SDA/SCL       ┌─────────────┐  │
│   │              ├──────────────────────► │  SFP Slot   │  │
│   │ Raspberry Pi │                         │  (Breakout) │  │
│   │    Pico      │      I²C SDA/SCL       │             │  │
│   │  (RP2040)    ├──────────────────────► │ SSD1306     │  │
│   │              │                         │ OLED 128×64 │  │
│   │              │      ADC GP26-28        │             │  │
│   │              ├──────────────────────► │  Joystick   │  │
│   └──────────────┘                         └─────────────┘  │
│                                                              │
│   Alimentação: USB 5V ou bateria Li-Ion via regulador 3.3V  │
└─────────────────────────────────────────────────────────────┘
```

| Componente | Especificação |
|---|---|
| Microcontrolador | Raspberry Pi Pico — RP2040, dual-core Cortex-M0+, 133 MHz |
| Display | OLED SSD1306, 128×64 px, I²C |
| Interface SFP | Breakout board com slot SFP, adaptação de nível lógico 3.3 V |
| Navegação | Joystick analógico 5 vias — ADC 12-bit |
| Build system | CMake 3.13+ · Pico SDK |
| Saída de firmware | `main.uf2` — drag-and-drop via USB BOOTSEL |

---

### Expansão planejada: outros protocolos

A arquitetura em camadas foi desenhada para crescer. Cada novo protocolo é adicionado como módulo independente na camada de Protocolo, sem tocar em display, navegação ou I²C.

```
HOJE                  PRÓXIMO                   FUTURO
─────                 ───────                   ──────
SFP / SFP+      ──►   QSFP / QSFP+       ──►   QSFP-DD / OSFP / CFP2
SFF-8472              SFF-8636                  CMIS
1G – 10G              40G – 100G                100G – 800G
1 lane óptico         4 lanes ópticos           8 lanes ópticos
2 endereços I²C       1 endereço I²C            Acesso por páginas (page-based)
✓ Implementado        Planejado                 Futuro

Para adicionar QSFP:   mkdir qsfp_8636/   → implementa qsfp_init() / qsfp_update()
Para adicionar CMIS:   mkdir cmis/        → implementa cmis_read_page() / cmis_update()
```

**Diferenças-chave dos protocolos futuros:**

| | SFF-8472 (atual) | SFF-8636 (QSFP) | CMIS (QSFP-DD) |
|---|---|---|---|
| Endereços I²C | 0x50 + 0x51 | 0x50 | 0x50 |
| Acesso | Direto (offset) | Direto + páginas | Páginas (banco + página) |
| Lanes | 1 | 4 | 8 |
| Potência por lane | ✓ | ✓ | ✓ |
| Velocidade típica | até 10G | até 100G | até 800G |

---

### Como compilar e gravar

```bash
# 1. Clonar o repositório
git clone https://github.com/embarcados-virtus-cc/optic-power-meter-portable
cd optic-power-meter-portable

# 2. Configurar o Pico SDK
export PICO_SDK_PATH=/caminho/para/pico-sdk   # Linux/macOS
# set PICO_SDK_PATH=C:\pico-sdk              # Windows

# 3. Compilar
mkdir build && cd build
cmake ..
make -j4

# 4. Gravar no Pico
# Segure BOOTSEL ao conectar o USB
# Copie build/main.uf2 para o volume "RPI-RP2" que aparecer
```

---

##  English

### What is this project?

This embedded device plugs directly into an SFP/SFP+ transceiver and displays all its internal data on a portable OLED screen — no computer, no proprietary software, no industrial test equipment.

**Accessible data:**

| Data | Unit | Source |
|---|---|---|
| Received optical power (Rx) | dBm / µW | A2h |
| Transmitted optical power (Tx) | dBm / µW | A2h |
| Internal temperature | °C | A2h |
| Supply voltage | V | A2h |
| Laser bias current | mA | A2h |
| Vendor name & part number | — | A0h |
| Wavelength | nm | A0h |
| Active alarms & warnings | flags | A2h |

---

### Why does this project exist?

SFP transceivers expose a standardized memory map (SFF-8472) over I²C with rich identification and diagnostic data. The problem: **there are almost no accessible embedded solutions** to query them in the field.

```
Existing options:
┌─────────────────────────────────────────┐
│  Industrial test equipment              │  High cost, not portable
│  PC software                            │  Requires cable and driver
│  Manual datasheet reading               │  Impractical in the field
└─────────────────────────────────────────┘

This project:
┌─────────────────────────────────────────┐
│  Raspberry Pi Pico + SFP breakout       │  < $20 in parts
│  Battery + portable OLED                │  Field-ready
│  Open source                            │  Extensible to QSFP, CMIS
└─────────────────────────────────────────┘
```

---

### Key concepts

#### What is an SFP transceiver?

SFP (Small Form-Factor Pluggable) is a compact module that converts electrical signals to optical (and back). It's the component you insert into switches, routers, OLTs, and ONUs to connect optical fibers.

```
          OPTICAL NETWORK
              │
    ┌─────────┴─────────┐
    │   LC optical fiber │
    └─────────┬─────────┘
              │  light (1310nm / 1550nm)
    ┌─────────▼─────────┐
    │    SFP Module     │  ← This project reads from here
    │  ┌─────────────┐  │
    │  │  Photodiode │  │  Rx: light → current
    │  │  VCSEL laser│  │  Tx: current → light
    │  │  MCU + EEPROM│ │  I²C: exposes internal data
    │  └─────────────┘  │
    └─────────┬─────────┘
              │  electrical signal SFI/SFF
    ┌─────────▼─────────┐
    │  Switch / Router  │
    └───────────────────┘
```

#### The SFF-8472 protocol

The SFF-8472 specification defines how the SFP module organizes its data in memory and exposes it over I²C. There are two addresses:

```
I²C bus
   │
   ├──► 0x50 (A0h) — Serial ID / Identification
   │         │
   │         ├── [bytes  0–19]  Type, connector, encoding, bitrate
   │         ├── [bytes 20–35]  Vendor name  (e.g. "FINISAR CORP.")
   │         ├── [bytes 40–55]  Vendor P/N   (e.g. "FTLX8571D3BCL")
   │         ├── [bytes 60–61]  Wavelength   (e.g. 850 nm)
   │         ├── [byte  63]     CC_BASE checksum
   │         ├── [bytes 64–94]  Extended ID
   │         │       └── [byte 92]  Diag. Monitoring Type ← enables DMI
   │         └── [byte  95]     CC_EXT checksum
   │
   └──► 0x51 (A2h) — Diagnostics / DMI
             │
             ├── [bytes  0–55]  Alarm & warning thresholds
             │       ├── Temp High/Low Alarm & Warning
             │       ├── VCC  High/Low Alarm & Warning
             │       ├── TX Bias High/Low Alarm & Warning
             │       ├── TX Power High/Low Alarm & Warning
             │       └── RX Power High/Low Alarm & Warning
             │
             ├── [bytes 56–91]  Real-time measurements
             │       ├── [56–57]  Temperature (°C)
             │       ├── [58–59]  VCC (V × 10000)
             │       ├── [60–61]  TX Bias (mA × 500)
             │       ├── [62–63]  TX Power (µW × 10000)
             │       └── [64–65]  RX Power (µW × 10000)
             │
             ├── [byte 110]    Data_Not_Ready flag ← checked before reading
             └── [bytes 96–127] Control & optional pages
```

#### Raw-to-physical unit conversion

```c
// Temperature: signed 16-bit two's complement, LSB = 1/256 °C
float temp_C = (int16_t)raw_temp / 256.0f;

// Supply voltage: unsigned 16-bit, LSB = 100 µV
float vcc_V = raw_vcc / 10000.0f;

// TX Bias current: unsigned 16-bit, LSB = 2 µA
float bias_mA = raw_bias / 500.0f;

// Optical power (RX/TX): unsigned 16-bit, LSB = 0.1 µW
float power_uW  = raw_power / 10.0f;
float power_dBm = 10.0f * log10f(power_uW / 1000.0f);
```

#### What is DMI?

DMI (Digital Monitoring Interface) is the feature that lets the transceiver expose calibrated measurements. Before reading A2h, the firmware checks if DMI is enabled at byte 92 of A0h:

```c
// Bit 6 of byte 92 = "Digital diagnostics implemented"
bool check_sfp_a2h_exists(sfp_t *sfp) {
    return (sfp->a0e.diag_monitoring_type >> 6) & 0x01;
}
```

---

### Firmware architecture

```
┌──────────────────────────────────────────────────────────────┐
│                       APPLICATION                             │
│                                                               │
│   menu/menu.c          main.c        Grafico_Experimental.py  │
│   ─────────────        ──────        ─────────────────────    │
│   7 OLED screens:      50 ms loop:   Offline analysis:        │
│   • Main menu          init I²C      Plot exported data       │
│   • Diagnostics        init OLED                              │
│   • Alarms             init joystick                          │
│   • Status             ┌──────────┐                           │
│   • Module info        │joystick  │                           │
│   • Monitoring         │SFP update│                           │
│   • Config             │render    │                           │
│                        └──────────┘                           │
├──────────────────────────────────────────────────────────────┤
│                        PROTOCOL                               │
│                                                               │
│   sfp_8472/                    I2C/            ssd1306/       │
│   ──────────                   ────            ────────       │
│   a0h.c · a0h.h                i2c.c           ssd1306.c      │
│   → A0h parser (128 bytes)     I²C HAL         OLED driver    │
│   a2h.c · a2h.h                abstraction                    │
│   → A2h parser (128 bytes)     joystick/                      │
│   → float conversion           JoystickPi.c                   │
│   sfp.c · sfp.h                5-way ADC driver               │
│   → init / update / checksums                                 │
├──────────────────────────────────────────────────────────────┤
│                  HARDWARE — RP2040 / Pico SDK                 │
│                                                               │
│      hardware_i2c       hardware_adc       hardware_pwm       │
│      I²C peripheral     12-bit ADC         Backlight PWM      │
└──────────────────────────────────────────────────────────────┘
```

**Execution flow:**

```
main()
  │
  ├─ ssd1306_Init()          → init OLED display over I²C
  ├─ joystickPi_init()       → configure ADC for joystick
  ├─ sfp_i2c_init(100 kHz)   → init I²C bus ← MUST come before reading SFP
  ├─ sleep_ms(2000)          → wait for SFP module to stabilise
  ├─ init_sfp_data()         → read A0h, validate checksums, check DMI
  │
  └─ while(true) [50 ms]
       ├─ process_joystick_input()   → detect direction / click
       ├─ update_system_data()       → read A0h + A2h over I²C
       └─ render_current_screen()    → draw screen on OLED
```

---

### Planned expansion: additional protocols

```
TODAY                 NEXT                      FUTURE
─────                 ────                      ──────
SFP / SFP+      ──►   QSFP / QSFP+       ──►   QSFP-DD / OSFP / CFP2
SFF-8472              SFF-8636                  CMIS
1G – 10G              40G – 100G                100G – 800G
1 optical lane        4 optical lanes           8 optical lanes
2 I²C addresses       1 I²C address             Page-based access
✓ Done                Planned                   Future

To add QSFP:   mkdir qsfp_8636/  → implement qsfp_init() / qsfp_update()
To add CMIS:   mkdir cmis/       → implement cmis_read_page() / cmis_update()
```

| | SFF-8472 (current) | SFF-8636 (QSFP) | CMIS (QSFP-DD) |
|---|---|---|---|
| I²C addresses | 0x50 + 0x51 | 0x50 | 0x50 |
| Access | Direct (offset) | Direct + pages | Pages (bank + page) |
| Lanes | 1 | 4 | 8 |
| Per-lane power | ✓ | ✓ | ✓ |
| Typical speed | up to 10G | up to 100G | up to 800G |

---

### How to build and flash

```bash
# 1. Clone the repository
git clone https://github.com/embarcados-virtus-cc/optic-power-meter-portable
cd optic-power-meter-portable

# 2. Set the Pico SDK path
export PICO_SDK_PATH=/path/to/pico-sdk   # Linux/macOS
# set PICO_SDK_PATH=C:\pico-sdk         # Windows

# 3. Build
mkdir build && cd build
cmake ..
make -j4

# 4. Flash to Pico
# Hold BOOTSEL while connecting USB
# Copy build/main.uf2 to the "RPI-RP2" volume that appears
```

---

**License:** MIT — see [`LICENSE`](LICENSE)
