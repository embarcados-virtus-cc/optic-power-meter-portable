#ifndef MENU_SFP_H
#define MENU_SFP_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "ssd1306/ssd1306.h"
#include "ssd1306/ssd1306_fonts.h"
#include "joystick/JoystickPi.h"
#include "sfp_8472/sfp_8472.h"

// ==================== DEFINIÇÕES GERAIS ====================
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define MAX_MENU_ITEMS 6
#define ITEM_HEIGHT 10
#define HEADER_HEIGHT 12
#define FOOTER_HEIGHT 8
#define DATA_UPDATE_INTERVAL_MS 1000
#define JOYSTICK_DEBOUNCE_MS 200
#define BUTTON_DEBOUNCE_MS 300
#define JOYSTICK_THRESHOLD 80
#define JOYSTICK_CENTER 2048
#define MAX_VISIBLE_ITEMS 4

// ==================== ESTRUTURAS DE DADOS ====================

/**
 * @brief Estado do sistema de menu
 */
typedef enum {
    STATE_MAIN_MENU,
    STATE_ALARMES,
    STATE_STATUS,
    STATE_DADOS_INFO,
    STATE_DIAGNOSTICO,
    STATE_CONFIG,
    STATE_MONITORAMENTO
} SystemState;

/**
 * @brief Estrutura para dados do módulo SFP
 */
typedef struct {
    char fabricante[20];
    char tipo[20];
    char serial[15];
    uint16_t comprimento_onda;
    uint16_t distancia_max;
    float temperatura;
    float tensao;
    float potencia_tx;
    float potencia_rx;
    float corrente_bias;
    uint8_t alarmes_ativos;
    uint16_t taxa_dados;
} SFP_Data;

/**
 * @brief Estrutura para itens do menu
 */
typedef struct {
    char label[20];
    char value[15];
    SystemState target_state;
} MenuItem;

/**
 * @brief Estrutura para controle do sistema
 */
typedef struct {
    SystemState current_state;
    SystemState previous_state;
    uint8_t current_selection;
    uint8_t scroll_offset;
    uint32_t last_joystick_move;
    uint32_t last_button_press;
    uint32_t last_data_update;
    SFP_Data sfp_data;
    sfp_a0h_base_t a0;
    bool joystick_enabled;
    uint8_t scroll_position;
} SystemControl;

// ==================== DECLARAÇÕES DE FUNÇÕES ====================

// Funções de inicialização
void init_sfp_data(void);
void update_sfp_data(void);

// Funções de desenho
void draw_header(const char* title);
void draw_footer(const char* instruction);
void draw_menu_item(uint8_t y_pos, const char* label, const char* value, 
                    bool is_selected, bool show_scroll_indicators);
void draw_menu_position_indicator(uint8_t current, uint8_t total);
void adjust_scroll_offset(void);

// Telas do sistema
void draw_main_menu(void);
void draw_alarmes_screen(void);
void draw_status_screen(void);
void draw_dados_info_screen(void);
void draw_diagnostico_screen(void);
void draw_config_screen(void);
void draw_monitoramento_screen(void);

// Controle do sistema
void process_joystick_input(void);
void update_system_data(void);
void render_current_screen(void);


//String do SFP(Converte informação do Módulo a0h para string)
const char* ext_compliance_to_string(sfp_extended_spec_compliance_code_t code);
const char* sfp_identifier_to_string(sfp_identifier_t id);
const char* sfp_compliance_byte3_to_string(const sfp_compliance_decoded_t *c);
const char* sfp_compliance_byte4_to_string(const sfp_compliance_decoded_t *c);
const char* sfp_compliance_byte5_to_string(const sfp_compliance_decoded_t *c);
const char* sfp_compliance_byte6_to_string(const sfp_compliance_decoded_t *c);
const char* sfp_compliance_byte7_to_string(const sfp_compliance_decoded_t *c);
const char* sfp_compliance_byte8_to_string(const sfp_compliance_decoded_t *c);
const char* sfp_compliance_byte9_to_string(const sfp_compliance_decoded_t *c);
const char* sfp_compliance_byte10_to_string(const sfp_compliance_decoded_t *c);
const char* sfp_encoding_to_string(sfp_encoding_codes_t encoding);
const char* sfp_om2_to_string(sfp_om2_length_status_t om2_status,uint16_t om2_length_m);







extern SystemControl system_ctrl;
#endif // MENU_SFP_H
