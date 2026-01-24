#include "menu.h"

// ==================== VARIÁVEIS GLOBAIS ====================
SystemControl system_ctrl = {
    .current_state = STATE_MAIN_MENU,
    .previous_state = STATE_MAIN_MENU,
    .current_selection = 0,
    .scroll_offset = 0,
    .last_joystick_move = 0,
    .last_button_press = 0,
    .last_data_update = 0,
    .sfp_data = {0},
    .a0 = {0},
    .joystick_enabled = true,
    .scroll_position = 0
};

// ==================== DADOS ESTÁTICOS ====================
static const char* FABRICANTES[] = {
    "CISCO", "JUNIPER", "HUAWEI", "FINISAR", "AVAGO",
    "INTEL", "BROADCOM", "DELL", "HP", "ARISTA"
};

static const char* TIPOS_MODULO[] = {
    "SFP-10G-SR", "SFP-10G-LR", "FP-10G-ER", "SFP-10G-ZR",
    "SFP-28G-SR", "SFP-28G-LR", "SFP+ 10G", "QSFP-40G-SR4",
    "QSFP-100G-SR4", "QSFP-DD-400G"
};

static const uint16_t COMPRIMENTOS_ONDA[] = {850, 1310, 1490, 1550, 1310, 1550};
static const uint16_t DISTANCIAS_MAX[] = {300, 10000, 40000, 100, 2000, 8000};
static const char SERIAL_NUMBERS[][12] = {
    "FNS12345678", "JNP87654321", "HWE11223344", "FIN55667788",
    "AVG99887766", "INT33445566", "BRD22334455", "DEL77889900",
    "HP00112233", "ARS44556677"
};

// ==================== FUNÇÕES AUXILIARES ====================

/**
 * @brief Inicializa dados do módulo SFP com valores aleatórios realistas
 */
void init_sfp_data(void) {
    uint32_t seed = to_ms_since_boot(get_absolute_time());
    srand(seed);
    
    int idx;
    
    // Fabricante
    idx = rand() % (sizeof(FABRICANTES) / sizeof(FABRICANTES[0]));
    strncpy(system_ctrl.sfp_data.fabricante, FABRICANTES[idx], 
            sizeof(system_ctrl.sfp_data.fabricante) - 1);
    
    // Tipo de módulo
    idx = rand() % (sizeof(TIPOS_MODULO) / sizeof(TIPOS_MODULO[0]));
    strncpy(system_ctrl.sfp_data.tipo, TIPOS_MODULO[idx],
            sizeof(system_ctrl.sfp_data.tipo) - 1);
    
    // Número de série
    idx = rand() % (sizeof(SERIAL_NUMBERS) / sizeof(SERIAL_NUMBERS[0]));
    strncpy(system_ctrl.sfp_data.serial, SERIAL_NUMBERS[idx],
            sizeof(system_ctrl.sfp_data.serial) - 1);
    
    // Comprimento de onda
    idx = rand() % (sizeof(COMPRIMENTOS_ONDA) / sizeof(COMPRIMENTOS_ONDA[0]));
    system_ctrl.sfp_data.comprimento_onda = COMPRIMENTOS_ONDA[idx];
    
    // Distância máxima
    idx = rand() % (sizeof(DISTANCIAS_MAX) / sizeof(DISTANCIAS_MAX[0]));
    system_ctrl.sfp_data.distancia_max = DISTANCIAS_MAX[idx];
    
    // Valores iniciais de operação
    system_ctrl.sfp_data.temperatura = 35.0f + (rand() % 100) / 10.0f;
    system_ctrl.sfp_data.tensao = 3.2f + (rand() % 150) / 100.0f;
    system_ctrl.sfp_data.potencia_tx = -2.0f - (rand() % 30) / 10.0f;
    system_ctrl.sfp_data.potencia_rx = -3.0f - (rand() % 40) / 10.0f;
    system_ctrl.sfp_data.corrente_bias = 30.0f + (rand() % 400) / 10.0f;
    system_ctrl.sfp_data.alarmes_ativos = rand() % 4;
    
    // Determina taxa de dados baseada no tipo
    if (strstr(system_ctrl.sfp_data.tipo, "10G") != NULL) {
        system_ctrl.sfp_data.taxa_dados = 10;
    } else if (strstr(system_ctrl.sfp_data.tipo, "28G") != NULL) {
        system_ctrl.sfp_data.taxa_dados = 28;
    } else if (strstr(system_ctrl.sfp_data.tipo, "40G") != NULL) {
        system_ctrl.sfp_data.taxa_dados = 40;
    } else if (strstr(system_ctrl.sfp_data.tipo, "100G") != NULL) {
        system_ctrl.sfp_data.taxa_dados = 100;
    } else if (strstr(system_ctrl.sfp_data.tipo, "400G") != NULL) {
        system_ctrl.sfp_data.taxa_dados = 400;
    } else {
        system_ctrl.sfp_data.taxa_dados = 1;
    }
}

/**
 * @brief Atualiza dados do SFP com variações realistas
 */
void update_sfp_data(void) {
    // Variação de temperatura (-0.2°C a +0.2°C)
    system_ctrl.sfp_data.temperatura += ((rand() % 5) - 2) * 0.1f;
    
    // Limites de temperatura operacional
    if (system_ctrl.sfp_data.temperatura < 20.0f) {
        system_ctrl.sfp_data.temperatura = 20.0f;
    }
    if (system_ctrl.sfp_data.temperatura > 70.0f) {
        system_ctrl.sfp_data.temperatura = 70.0f;
    }
    
    // Variação de tensão
    system_ctrl.sfp_data.tensao += ((rand() % 3) - 1) * 0.01f;
    if (system_ctrl.sfp_data.tensao < 3.0f) system_ctrl.sfp_data.tensao = 3.0f;
    if (system_ctrl.sfp_data.tensao > 3.6f) system_ctrl.sfp_data.tensao = 3.6f;
    
    // Variações de potência e corrente
    system_ctrl.sfp_data.potencia_tx += ((rand() % 5) - 2) * 0.1f;
    system_ctrl.sfp_data.potencia_rx += ((rand() % 5) - 2) * 0.1f;
    system_ctrl.sfp_data.corrente_bias += ((rand() % 5) - 2) * 0.1f;
    
    // Geração ocasional de alarmes
    if (rand() % 50 == 0) {
        system_ctrl.sfp_data.alarmes_ativos = (system_ctrl.sfp_data.alarmes_ativos + 1) % 5;
    }
}

// ==================== FUNÇÕES DE DESENHO ====================

/**
 * @brief Desenha cabeçalho da tela
 * @param title Título a ser exibido
 */
void draw_header(const char* title) {
    // Linha separadora
    ssd1306_Line(0, HEADER_HEIGHT - 2, DISPLAY_WIDTH - 1, HEADER_HEIGHT - 2, White);
    
    // Centraliza e desenha o título
    int title_len = strlen(title) * 6;
    int x_pos = (DISPLAY_WIDTH - title_len) / 2;
    
    if (x_pos < 2) x_pos = 2;
    
    ssd1306_SetCursor(x_pos, 0);
    ssd1306_WriteString(title, Font_6x8, White);
}

/**
 * @brief Desenha rodapé com instruções
 */
void draw_footer(const char* instruction) {
    uint8_t y_pos = DISPLAY_HEIGHT - FOOTER_HEIGHT + 2;
    
    // Linha separadora
    ssd1306_Line(0, DISPLAY_HEIGHT - FOOTER_HEIGHT, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - FOOTER_HEIGHT, White);
    
    // Instrução centralizada
    int instr_len = strlen(instruction) * 6;
    int x_pos = (DISPLAY_WIDTH - instr_len) / 2;
    
    if (x_pos < 2) x_pos = 2;
    
    ssd1306_SetCursor(x_pos, y_pos);
    ssd1306_WriteString(instruction, Font_6x8, White);
}

/**
 * @brief Desenha um item do menu
 */
void draw_menu_item(uint8_t y_pos, const char* label, const char* value, 
                    bool is_selected, bool show_scroll_indicators) {
    if (show_scroll_indicators) {
        // Indicador de rolagem para cima
        if (system_ctrl.scroll_offset > 0) {
            ssd1306_SetCursor(DISPLAY_WIDTH - 8, HEADER_HEIGHT + 1);
            ssd1306_WriteString("^", Font_6x8, White);
        }
        
        // Indicador de rolagem para baixo
        uint8_t total_items = 7; // Para menu principal
        if (system_ctrl.scroll_offset + MAX_VISIBLE_ITEMS < total_items) {
            ssd1306_SetCursor(DISPLAY_WIDTH - 8, DISPLAY_HEIGHT - FOOTER_HEIGHT - 8);
            ssd1306_WriteString("v", Font_6x8, White);
        }
    }
    
    if (is_selected) {
        // Fundo invertido para item selecionado
        ssd1306_FillRectangle(0, y_pos - 1, DISPLAY_WIDTH - 1, 
                              y_pos + ITEM_HEIGHT - 1, White);
        
        // Seta de seleção
        ssd1306_SetCursor(2, y_pos);
        ssd1306_WriteString(">", Font_6x8, Black);
        
        // Label em cor invertida
        ssd1306_SetCursor(10, y_pos);
        ssd1306_WriteString(label, Font_6x8, Black);
        
        // Valor (se existir)
        if (value != NULL && strlen(value) > 0) {
            int label_width = strlen(label) * 6 + 12;
            if (label_width < DISPLAY_WIDTH - 40) {
                ssd1306_SetCursor(label_width, y_pos);
                ssd1306_WriteString(value, Font_6x8, Black);
            }
        }
    } else {
        // Item não selecionado
        ssd1306_SetCursor(12, y_pos);
        ssd1306_WriteString(label, Font_6x8, White);
        
        // Valor (se existir)
        if (value != NULL && strlen(value) > 0) {
            int label_width = strlen(label) * 6 + 14;
            if (label_width < DISPLAY_WIDTH - 40) {
                ssd1306_SetCursor(label_width, y_pos);
                ssd1306_WriteString(value, Font_6x8, White);
            }
        }
    }
}

/**
 * @brief Desenha indicador de posição no menu
 */
void draw_menu_position_indicator(uint8_t current, uint8_t total) {
    char indicator[20];
    snprintf(indicator, sizeof(indicator), "%d/%d", current + 1, total);
    
    int indicator_len = strlen(indicator) * 6;
    int x_pos = DISPLAY_WIDTH - indicator_len - 4;
    
    ssd1306_SetCursor(x_pos, DISPLAY_HEIGHT - FOOTER_HEIGHT + 2);
    ssd1306_WriteString(indicator, Font_6x8, White);
}

/**
 * @brief Ajusta o offset de rolagem baseado na seleção atual
 */
void adjust_scroll_offset(void) {
    // Se o item selecionado está acima do offset visível
    if (system_ctrl.current_selection < system_ctrl.scroll_offset) {
        system_ctrl.scroll_offset = system_ctrl.current_selection;
    }
    // Se o item selecionado está abaixo do offset visível
    else if (system_ctrl.current_selection >= system_ctrl.scroll_offset + MAX_VISIBLE_ITEMS) {
        system_ctrl.scroll_offset = system_ctrl.current_selection - MAX_VISIBLE_ITEMS + 1;
    }
}

// ==================== TELAS DO SISTEMA ====================

/**
 * @brief Desenha tela do menu principal
 */
void draw_main_menu(void) {
    ssd1306_Fill(Black);
    draw_header("MENU SFP/SFP+");
    draw_footer("ENTER:Selecionar  UP/DOWN:Rolar");
    
    // Itens do menu
    MenuItem menu_items[MAX_MENU_ITEMS] = {
        {"ALARMES", "", STATE_ALARMES},
        {"STATUS", "", STATE_STATUS},
        {"INFO COMPLETA", "", STATE_DADOS_INFO},
        {"DIAGNOSTICO", "", STATE_DIAGNOSTICO},
        {"CONFIG", "", STATE_CONFIG},
        {"MONITORAMENTO", "", STATE_MONITORAMENTO}
    };
    
    // Preenche valores dinâmicos
    char temp_buffer[15];
    
    // Número de alarmes ativos
    snprintf(temp_buffer, sizeof(temp_buffer), "%d", system_ctrl.sfp_data.alarmes_ativos);
    strncpy(menu_items[0].value, temp_buffer, sizeof(menu_items[0].value) - 1);
    
    // Temperatura atual
    snprintf(temp_buffer, sizeof(temp_buffer), "%.1fC", system_ctrl.sfp_data.temperatura);
    strncpy(menu_items[1].value, temp_buffer, sizeof(menu_items[1].value) - 1);
    
    // Taxa de dados
    snprintf(temp_buffer, sizeof(temp_buffer), "%dG", system_ctrl.sfp_data.taxa_dados);
    strncpy(menu_items[2].value, temp_buffer, sizeof(menu_items[2].value) - 1);
    
    // Ajusta offset de rolagem
    adjust_scroll_offset();
    
    // Desenha apenas os itens visíveis
    uint8_t start_y = HEADER_HEIGHT + 2;
    
    for (uint8_t i = 0; i < MAX_VISIBLE_ITEMS; i++) {
        uint8_t item_index = system_ctrl.scroll_offset + i;
        if (item_index >= MAX_MENU_ITEMS) break;
        
        uint8_t y_pos = start_y + (i * ITEM_HEIGHT);
        bool is_selected = (item_index == system_ctrl.current_selection);
        
        draw_menu_item(y_pos, menu_items[item_index].label, 
                      menu_items[item_index].value, is_selected, true);
    }
    
    // Indicador de posição
    draw_menu_position_indicator(system_ctrl.current_selection, MAX_MENU_ITEMS);
}

/**
 * @brief Desenha tela de alarmes com rolagem
 */
void draw_alarmes_screen(void) {
    ssd1306_Fill(Black);
    draw_header("ALARMES ATIVOS");
    draw_footer("UP/DOWN:Rolar  ENTER:Menu");
    
    uint8_t start_y = HEADER_HEIGHT + 2;
    
    if (system_ctrl.sfp_data.alarmes_ativos == 0) {
        // Nenhum alarme
        ssd1306_SetCursor(30, start_y + 10);
        ssd1306_WriteString("SEM ALARMES", Font_7x10, White);
        
        ssd1306_SetCursor(25, start_y + 25);
        ssd1306_WriteString("Sistema OK", Font_6x8, White);
    } else {
        // Lista de alarmes com possibilidade de rolagem
        const char* alarmes[] = {
            "ALTA TEMPERATURA (>70C)",
            "BAIXA POTENCIA TX (<-8dBm)",
            "ALTA POTENCIA RX (>-3dBm)", 
            "TENSAO FORA LIMITE",
            "LASER DESLIGADO",
            "FALHA NA FIBRA",
            "PERDA DE SINAL RX",
            "TEMPERATURA BAIXA (<0C)",
            "CORRENTE BIAS ALTA (>100mA)",
            "FALHA NO MÓDULO"
        };
        
        uint8_t max_alarmes = system_ctrl.sfp_data.alarmes_ativos;
        if (max_alarmes > 10) max_alarmes = 10;
        
        // Ajusta posição de rolagem
        if (system_ctrl.scroll_position >= max_alarmes) {
            system_ctrl.scroll_position = 0;
        }
        
        // Mostra indicadores de rolagem se necessário
        if (max_alarmes > MAX_VISIBLE_ITEMS) {
            if (system_ctrl.scroll_position > 0) {
                ssd1306_SetCursor(DISPLAY_WIDTH - 8, HEADER_HEIGHT + 1);
                ssd1306_WriteString("^", Font_6x8, White);
            }
            if (system_ctrl.scroll_position + MAX_VISIBLE_ITEMS < max_alarmes) {
                ssd1306_SetCursor(DISPLAY_WIDTH - 8, DISPLAY_HEIGHT - FOOTER_HEIGHT - 8);
                ssd1306_WriteString("v", Font_6x8, White);
            }
        }
        
        // Desenha alarmes visíveis
        for (uint8_t i = 0; i < MAX_VISIBLE_ITEMS; i++) {
            uint8_t alarme_index = system_ctrl.scroll_position + i;
            if (alarme_index >= max_alarmes) break;
            
            uint8_t y_pos = start_y + (i * ITEM_HEIGHT);
            
            // Ícone de alarme
            ssd1306_SetCursor(5, y_pos);
            ssd1306_WriteString("!", Font_7x10, White);
            
            // Texto do alarme
            ssd1306_SetCursor(20, y_pos);
            if (strlen(alarmes[alarme_index]) > 15) {
                // Se for muito longo, corta
                char buffer[20];
                strncpy(buffer, alarmes[alarme_index], 16);
                buffer[16] = '\0';
                strcat(buffer, "...");
                ssd1306_WriteString(buffer, Font_6x8, White);
            } else {
                ssd1306_WriteString(alarmes[alarme_index], Font_6x8, White);
            }
        }
        
        // Mostra contador
        char counter[20];
        snprintf(counter, sizeof(counter), "%d/%d", system_ctrl.scroll_position + 1, max_alarmes);
        int counter_len = strlen(counter) * 6;
        ssd1306_SetCursor(DISPLAY_WIDTH - counter_len - 5, DISPLAY_HEIGHT - FOOTER_HEIGHT + 2);
        ssd1306_WriteString(counter, Font_6x8, White);
    }
}

/**
 * @brief Desenha tela de status do sistema com rolagem
 */
void draw_status_screen(void) {
    ssd1306_Fill(Black);
    draw_header("STATUS ATUAL");
    draw_footer("UP/DOWN:Rolar  ENTER:Menu");
    
    // Array de dados de status
    char status_items[10][30];
    
    snprintf(status_items[0], sizeof(status_items[0]), "Temp:    %.1f C", system_ctrl.sfp_data.temperatura);
    snprintf(status_items[1], sizeof(status_items[1]), "Tensao:  %.2f V", system_ctrl.sfp_data.tensao);
    snprintf(status_items[2], sizeof(status_items[2]), "Pot TX:  %.1f dBm", system_ctrl.sfp_data.potencia_tx);
    snprintf(status_items[3], sizeof(status_items[3]), "Pot RX:  %.1f dBm", system_ctrl.sfp_data.potencia_rx);
    snprintf(status_items[4], sizeof(status_items[4]), "Bias:    %.1f mA", system_ctrl.sfp_data.corrente_bias);
    snprintf(status_items[5], sizeof(status_items[5]), "Taxa:    %d Gbps", system_ctrl.sfp_data.taxa_dados);
    snprintf(status_items[6], sizeof(status_items[6]), "Alarmes: %d", system_ctrl.sfp_data.alarmes_ativos);
    snprintf(status_items[7], sizeof(status_items[7]), "Serial:  %s", system_ctrl.sfp_data.serial);
    snprintf(status_items[8], sizeof(status_items[8]), "Fabric:  %s", system_ctrl.sfp_data.fabricante);
    snprintf(status_items[9], sizeof(status_items[9]), "Tipo:    %s", system_ctrl.sfp_data.tipo);
    
    // Mostra indicadores de rolagem se necessário
    if (system_ctrl.scroll_position > 0) {
        ssd1306_SetCursor(DISPLAY_WIDTH - 8, HEADER_HEIGHT + 1);
        ssd1306_WriteString("^", Font_6x8, White);
    }
    if (system_ctrl.scroll_position + MAX_VISIBLE_ITEMS < 10) {
        ssd1306_SetCursor(DISPLAY_WIDTH - 8, DISPLAY_HEIGHT - FOOTER_HEIGHT - 8);
        ssd1306_WriteString("v", Font_6x8, White);
    }
    
    uint8_t start_y = HEADER_HEIGHT + 2;
    
    // Desenha itens visíveis
    for (uint8_t i = 0; i < MAX_VISIBLE_ITEMS; i++) {
        uint8_t item_index = system_ctrl.scroll_position + i;
        if (item_index >= 10) break;
        
        uint8_t y_pos = start_y + (i * ITEM_HEIGHT);
        ssd1306_SetCursor(4, y_pos);
        ssd1306_WriteString(status_items[item_index], Font_6x8, White);
    }
    
    // Mostra contador
    char counter[20];
    snprintf(counter, sizeof(counter), "%d/%d", system_ctrl.scroll_position + 1, 10);
    int counter_len = strlen(counter) * 6;
    ssd1306_SetCursor(DISPLAY_WIDTH - counter_len - 5, DISPLAY_HEIGHT - FOOTER_HEIGHT + 2);
    ssd1306_WriteString(counter, Font_6x8, White);
}

/**
 * @brief Desenha tela unificada de dados e informações técnicas com rolagem
 */
void draw_dados_info_screen(void) {
    ssd1306_Fill(Black);
    draw_header("INFO. COMPLETA");
    draw_footer("UP/DOWN:Rolar  ENTER:Menu");
    
    // Array de todas as informações
    char info_items[15][35];
    
    // Seção 1: Dados básicos do módulo
    snprintf(info_items[0], sizeof(info_items[0]), "Vendor: %s", 
             system_ctrl.a0.vendor_name);
    snprintf(info_items[1], sizeof(info_items[1]), "MODELO: %s", 
             system_ctrl.sfp_data.tipo);
    snprintf(info_items[2], sizeof(info_items[2]), "SERIAL: %s", 
             system_ctrl.sfp_data.serial);
    //snprintf(info_items[3], sizeof(info_items[3]), "TAXA DADOS: %d Gbps", system_ctrl.sfp_data.taxa_dados);
    
    snprintf(info_items[3], sizeof(info_items[3]), "Ext.Spec: %s", ext_compliance_to_string(system_ctrl.a0.ext_compliance)); /*Byte 36*/
    snprintf(info_items[4], sizeof(info_items[4]), "Ident: %s",sfp_identifier_to_string(system_ctrl.a0.identifier)); /*Byte 0*/
    snprintf(info_items[5], sizeof(info_items[5]), "Eth/InfB:%s",sfp_compliance_byte3_to_string(&system_ctrl.a0.dc)); /*Byte 3*/
    snprintf(info_items[6], sizeof(info_items[6]), "Escon/Sonet:%s",sfp_compliance_byte4_to_string(&system_ctrl.a0.dc)); /*Byte 4*/
    snprintf(info_items[7], sizeof(info_items[7]), "Sonet:%s",sfp_compliance_byte5_to_string(&system_ctrl.a0.dc)); /*Byte 5*/
    snprintf(info_items[8], sizeof(info_items[8]), "Eth:%s",sfp_compliance_byte6_to_string(&system_ctrl.a0.dc)); /*Byte 6*/
    snprintf(info_items[9], sizeof(info_items[9]), "FbrCh_T:%s",sfp_compliance_byte7_to_string(&system_ctrl.a0.dc)); /*Byte 7*/
    snprintf(info_items[10], sizeof(info_items[10]),"FbrCh_CT:%s",sfp_compliance_byte8_to_string(&system_ctrl.a0.dc)); /*Byte 8*/
    snprintf(info_items[11], sizeof(info_items[11]), "Fbr_TM:%s",sfp_compliance_byte9_to_string(&system_ctrl.a0.dc)); /*Byte 9*/
    snprintf(info_items[12], sizeof(info_items[12]), "Fbr_S:%s",sfp_compliance_byte10_to_string(&system_ctrl.a0.dc)); /*Byte 10*/
    snprintf(info_items[13], sizeof(info_items[13]), "Enc:%s",sfp_encoding_to_string(system_ctrl.a0.encoding)); /*Byte 11*/
    snprintf(info_items[14], sizeof(info_items[14]), "OM2:%s",sfp_om2_to_string(system_ctrl.a0.om2_status,system_ctrl.a0.om2_length_m)); /*Byte 16*/
    
    
    
    
    
    
    
    
    // Seção 2: Especificações técnicas
    //snprintf(info_items[4], sizeof(info_items[4]), "COMPR. ONDA: %d nm", system_ctrl.sfp_data.comprimento_onda);
    //snprintf(info_items[5], sizeof(info_items[5]), "DIST. MAX: %d m",        system_ctrl.sfp_data.distancia_max);
    
    /*const char* fibra_tipo = (system_ctrl.sfp_data.comprimento_onda == 850) ? 
                             "MMF (OM3/OM4)" : "SMF";
    snprintf(info_items[6], sizeof(info_items[6]), "TIPO FIBRA: %s", fibra_tipo);*/
    
    // Seção 3: Parâmetros operacionais atuais
   /* snprintf(info_items[7], sizeof(info_items[7]), "TEMP ATUAL: %.1f C", 
             system_ctrl.sfp_data.temperatura);
    snprintf(info_items[8], sizeof(info_items[8]), "TENSAO ATUAL: %.2f V", 
             system_ctrl.sfp_data.tensao);
    snprintf(info_items[9], sizeof(info_items[9]), "POT TX: %.1f dBm", 
             system_ctrl.sfp_data.potencia_tx);
    snprintf(info_items[10], sizeof(info_items[10]), "POT RX: %.1f dBm", 
             system_ctrl.sfp_data.potencia_rx);
    snprintf(info_items[11], sizeof(info_items[11]), "BIAS: %.1f mA", 
             system_ctrl.sfp_data.corrente_bias);
    
    // Seção 4: Especificações técnicas
    strcpy(info_items[12], "TEMP OPER: 0 a 70C");
    strcpy(info_items[13], "TENSAO: 3.3V +/-10%");
    strcpy(info_items[14], "CONECTOR: LC/UPC");*/
    
    const uint8_t total_items = 15;
    
    // Mostra indicadores de rolagem se necessário
    if (system_ctrl.scroll_position > 0) {
        ssd1306_SetCursor(DISPLAY_WIDTH - 8, HEADER_HEIGHT + 1);
        ssd1306_WriteString("^", Font_6x8, White);
    }
    if (system_ctrl.scroll_position + MAX_VISIBLE_ITEMS < total_items) {
        ssd1306_SetCursor(DISPLAY_WIDTH - 8, DISPLAY_HEIGHT - FOOTER_HEIGHT - 8);
        ssd1306_WriteString("v", Font_6x8, White);
    }
    
    uint8_t start_y = HEADER_HEIGHT + 2;
    
    // Desenha categorias e separadores visuais
    uint8_t visible_count = 0;
    for (uint8_t i = 0; i < MAX_VISIBLE_ITEMS; i++) {
        uint8_t item_index = system_ctrl.scroll_position + i;
        if (item_index >= total_items) break;
        
        uint8_t y_pos = start_y + (i * ITEM_HEIGHT);
        
        // Adiciona ícones ou marcadores para diferentes seções
        if (item_index == 0 || item_index == 4 || item_index == 7 || item_index == 12) {
            // Linha separadora para seções
            ssd1306_Line(0, y_pos - 1, DISPLAY_WIDTH - 1, y_pos - 1, White);
            
            // Ícone para a seção
            if (item_index == 0) {
                ssd1306_SetCursor(2, y_pos);
                ssd1306_WriteString("D", Font_6x8, White);
            } else if (item_index == 4) {
                ssd1306_SetCursor(2, y_pos);
                ssd1306_WriteString("E", Font_6x8, White);
            } else if (item_index == 7) {
                ssd1306_SetCursor(2, y_pos);
                ssd1306_WriteString("O", Font_6x8, White);
            } else if (item_index == 12) {
                ssd1306_SetCursor(2, y_pos);
                ssd1306_WriteString("T", Font_6x8, White);
            }
            
            // Texto da informação
            ssd1306_SetCursor(12, y_pos);
            ssd1306_WriteString(info_items[item_index], Font_6x8, White);
        } else {
            // Informações normais
            ssd1306_SetCursor(12, y_pos);
            ssd1306_WriteString(info_items[item_index], Font_6x8, White);
        }
        visible_count++;
    }
    
    // Se não há itens suficientes para preencher a tela, mostra mensagem
    if (visible_count == 0) {
        ssd1306_SetCursor(20, start_y + 10);
        ssd1306_WriteString("SEM INFORMACOES", Font_7x10, White);
    }
    
    // Mostra contador de posição
    char counter[20];
    snprintf(counter, sizeof(counter), "%d/%d", 
             system_ctrl.scroll_position + 1, total_items);
    int counter_len = strlen(counter) * 6;
    ssd1306_SetCursor(DISPLAY_WIDTH - counter_len - 5, 
                      DISPLAY_HEIGHT - FOOTER_HEIGHT + 2);
    ssd1306_WriteString(counter, Font_6x8, White);
}

/**
 * @brief Desenha tela de diagnóstico
 */
void draw_diagnostico_screen(void) {
    ssd1306_Fill(Black);
    draw_header("DIAGNOSTICO");
    draw_footer("ENTER:Voltar ao Menu");
    
    uint8_t start_y = HEADER_HEIGHT + 10;
    
    // Título da análise
    ssd1306_SetCursor(25, start_y);
    ssd1306_WriteString("ANALISE SINAL", Font_7x10, White);
    
    // Status do sinal
    ssd1306_SetCursor(10, start_y + 15);
    if (system_ctrl.sfp_data.potencia_rx < -30.0f) {
        ssd1306_WriteString("SINAL MUITO FRACO", Font_6x8, White);
    } else if (system_ctrl.sfp_data.potencia_rx < -20.0f) {
        ssd1306_WriteString("SINAL FRACO", Font_6x8, White);
    } else if (system_ctrl.sfp_data.potencia_rx > 0.0f) {
        ssd1306_WriteString("SINAL ALTO", Font_6x8, White);
    } else {
        ssd1306_WriteString("SINAL NORMAL", Font_6x8, White);
    }
    
    // BER estimado
    ssd1306_SetCursor(10, start_y + 25);
    ssd1306_WriteString("BER: < 1e-12", Font_6x8, White);
    
    // Link status
    ssd1306_SetCursor(10, start_y + 35);
    if (system_ctrl.sfp_data.potencia_rx > -30.0f) {
        ssd1306_WriteString("LINK: UP", Font_6x8, White);
    } else {
        ssd1306_WriteString("LINK: DOWN", Font_6x8, White);
    }
    
    // Qualidade do link
    ssd1306_SetCursor(10, start_y + 45);
    if (system_ctrl.sfp_data.alarmes_ativos == 0) {
        ssd1306_WriteString("QUALIDADE: BOM", Font_6x8, White);
    } else {
        ssd1306_WriteString("QUALIDADE: BAIXA", Font_6x8, White);
    }
}

/**
 * @brief Desenha tela de configuração com rolagem
 */
void draw_config_screen(void) {
    ssd1306_Fill(Black);
    draw_header("CONFIGURACAO");
    draw_footer("ENTER:Selecionar  UP/DOWN:Rolar");
    
    uint8_t start_y = HEADER_HEIGHT + 2;
    
    const char* config_items[] = {
        "Reset Contadores",
        "Limiar Alarmes",
        "Formato Dados",
        "Idioma",
        "Brilho Display",
        "Timeout Auto",
        "Log Nivel",
        "Backup Config",
        "Restore Config",
        "Voltar ao Menu"
    };
    
    // Ajusta offset de rolagem
    adjust_scroll_offset();
    
    // Desenha apenas os itens visíveis
    for (uint8_t i = 0; i < MAX_VISIBLE_ITEMS; i++) {
        uint8_t item_index = system_ctrl.scroll_offset + i;
        if (item_index >= 10) break;
        
        uint8_t y_pos = start_y + (i * ITEM_HEIGHT);
        bool is_selected = (item_index == system_ctrl.current_selection);
        
        draw_menu_item(y_pos, config_items[item_index], "", is_selected, true);
    }
    
    draw_menu_position_indicator(system_ctrl.current_selection, 10);
}

/**
 * @brief Desenha tela de monitoramento
 */
void draw_monitoramento_screen(void) {
    ssd1306_Fill(Black);
    draw_header("MONITORAMENTO");
    draw_footer("ENTER:Voltar ao Menu");
    
    uint8_t start_y = HEADER_HEIGHT + 2;
    char buffer[15];
    
    // Gráfico de temperatura
    ssd1306_SetCursor(4, start_y);
    ssd1306_WriteString("Temperatura:", Font_6x8, White);
    
    // Barra de temperatura (0-100%)
    uint8_t temp_bar = (uint8_t)((system_ctrl.sfp_data.temperatura - 20.0f) / 50.0f * 100);
    if (temp_bar > 100) temp_bar = 100;
    ssd1306_FillRectangle(10, start_y + 12, 10 + temp_bar, start_y + 18, White);
    ssd1306_DrawRectangle(9, start_y + 11, 111, start_y + 19, White);
    
    // Valor da temperatura
    snprintf(buffer, sizeof(buffer), "%.1fC", system_ctrl.sfp_data.temperatura);
    ssd1306_SetCursor(115, start_y);
    ssd1306_WriteString(buffer, Font_6x8, White);
    
    // Gráfico de potência RX
    ssd1306_SetCursor(4, start_y + 25);
    ssd1306_WriteString("Potencia RX:", Font_6x8, White);
    
    // Barra de potência (-40dBm a 0dBm)
    uint8_t power_bar = (uint8_t)((system_ctrl.sfp_data.potencia_rx + 40.0f) / 40.0f * 100);
    if (power_bar > 100) power_bar = 100;
    ssd1306_FillRectangle(10, start_y + 37, 10 + power_bar, start_y + 43, White);
    ssd1306_DrawRectangle(9, start_y + 36, 111, start_y + 44, White);
    
    // Valor da potência
    snprintf(buffer, sizeof(buffer), "%.1fdBm", system_ctrl.sfp_data.potencia_rx);
    ssd1306_SetCursor(115, start_y + 25);
    ssd1306_WriteString(buffer, Font_6x8, White);
    
    // Status geral
    ssd1306_SetCursor(25, start_y + 50);
    if (system_ctrl.sfp_data.alarmes_ativos == 0) {
        ssd1306_WriteString("SISTEMA OK", Font_6x8, White);
    } else {
        ssd1306_WriteString("VERIFICAR ALARMES", Font_6x8, White);
    }
}

// ==================== CONTROLE DO JOYSTICK ====================

/**
 * @brief Processa a entrada do joystick para navegação
 */
void process_joystick_input(void) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    if (!system_ctrl.joystick_enabled) {
        return;
    }
    
    // Lê o estado atual do joystick
    joystick_state_t joystick = joystickPi_read();
    
    // Processa movimento no eixo Y (navegação vertical)
    if (current_time - system_ctrl.last_joystick_move > JOYSTICK_DEBOUNCE_MS) {
        int16_t y_mapped = joystickPi_map_value(joystick.y, 0, 4095, -100, 100);
        
        // Movimento para cima
        if (y_mapped < -JOYSTICK_THRESHOLD) {
            system_ctrl.last_joystick_move = current_time;
            
            if (system_ctrl.current_state == STATE_MAIN_MENU) {
                if (system_ctrl.current_selection > 0) {
                    system_ctrl.current_selection--;
                } else {
                    system_ctrl.current_selection = MAX_MENU_ITEMS - 1;
                }
                adjust_scroll_offset();
            }
            else if (system_ctrl.current_state == STATE_CONFIG) {
                if (system_ctrl.current_selection > 0) {
                    system_ctrl.current_selection--;
                } else {
                    system_ctrl.current_selection = 9;
                }
                adjust_scroll_offset();
            }
            else if (system_ctrl.current_state == STATE_ALARMES) {
                if (system_ctrl.scroll_position > 0) {
                    system_ctrl.scroll_position--;
                }
            }
            else if (system_ctrl.current_state == STATE_STATUS) {
                if (system_ctrl.scroll_position > 0) {
                    system_ctrl.scroll_position--;
                }
            }
            else if (system_ctrl.current_state == STATE_DADOS_INFO) {
                if (system_ctrl.scroll_position > 0) {
                    system_ctrl.scroll_position--;
                }
            }
        }
        // Movimento para baixo
        else if (y_mapped > JOYSTICK_THRESHOLD) {
            system_ctrl.last_joystick_move = current_time;
            
            if (system_ctrl.current_state == STATE_MAIN_MENU) {
                system_ctrl.current_selection = (system_ctrl.current_selection + 1) % MAX_MENU_ITEMS;
                adjust_scroll_offset();
            }
            else if (system_ctrl.current_state == STATE_CONFIG) {
                system_ctrl.current_selection = (system_ctrl.current_selection + 1) % 10;
                adjust_scroll_offset();
            }
            else if (system_ctrl.current_state == STATE_ALARMES) {
                uint8_t max_alarmes = system_ctrl.sfp_data.alarmes_ativos;
                if (max_alarmes > 10) max_alarmes = 10;
                if (system_ctrl.scroll_position + MAX_VISIBLE_ITEMS < max_alarmes) {
                    system_ctrl.scroll_position++;
                }
            }
            else if (system_ctrl.current_state == STATE_STATUS) {
                if (system_ctrl.scroll_position + MAX_VISIBLE_ITEMS < 10) {
                    system_ctrl.scroll_position++;
                }
            }
            else if (system_ctrl.current_state == STATE_DADOS_INFO) {
                const uint8_t total_items = 15;
                if (system_ctrl.scroll_position + MAX_VISIBLE_ITEMS < total_items) {
                    system_ctrl.scroll_position++;
                }
            }
        }
    }
    
    // Processa botão (seleção/confirmação)
    if (current_time - system_ctrl.last_button_press > BUTTON_DEBOUNCE_MS) {
        if (joystick.button) {
            system_ctrl.last_button_press = current_time;
            
            if (system_ctrl.current_state == STATE_MAIN_MENU) {
                switch (system_ctrl.current_selection) {
                    case 0: system_ctrl.current_state = STATE_ALARMES; break;
                    case 1: system_ctrl.current_state = STATE_STATUS; break;
                    case 2: system_ctrl.current_state = STATE_DADOS_INFO; break;
                    case 3: system_ctrl.current_state = STATE_DIAGNOSTICO; break;
                    case 4: system_ctrl.current_state = STATE_CONFIG; break;
                    case 5: system_ctrl.current_state = STATE_MONITORAMENTO; break;
                }
                system_ctrl.current_selection = 0;
                system_ctrl.scroll_position = 0;
                system_ctrl.scroll_offset = 0;
            }
            else if (system_ctrl.current_state == STATE_CONFIG) {
                if (system_ctrl.current_selection == 9) {
                    system_ctrl.current_state = STATE_MAIN_MENU;
                    system_ctrl.current_selection = 4;
                    system_ctrl.scroll_offset = 0;
                }
            }
            else {
                system_ctrl.previous_state = system_ctrl.current_state;
                system_ctrl.current_state = STATE_MAIN_MENU;
                system_ctrl.scroll_position = 0;
            }
        }
    }
}

/**
 * @brief Atualiza dados do sistema periodicamente
 */
void update_system_data(void) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Atualiza dados do SFP periodicamente
    if (current_time - system_ctrl.last_data_update > DATA_UPDATE_INTERVAL_MS) {
        update_sfp_data();
        system_ctrl.last_data_update = current_time;
    }
}

// ==================== GERENCIAMENTO DO SISTEMA ====================

/**
 * @brief Renderiza a tela atual
 */
void render_current_screen(void) {
    switch (system_ctrl.current_state) {
        case STATE_MAIN_MENU:
            draw_main_menu();
            break;
        case STATE_ALARMES:
            draw_alarmes_screen();
            break;
        case STATE_STATUS:
            draw_status_screen();
            break;
        case STATE_DADOS_INFO:
            draw_dados_info_screen();
            break;
        case STATE_DIAGNOSTICO:
            draw_diagnostico_screen();
            break;
        case STATE_CONFIG:
            draw_config_screen();
            break;
        case STATE_MONITORAMENTO:
            draw_monitoramento_screen();
            break;
        default:
            draw_main_menu();
            break;
    }
}

// ====================== STRING DO SFP/SFP+ =====================================


const char* ext_compliance_to_string(sfp_extended_spec_compliance_code_t code) {
    switch (code) {
        case EXT_SPEC_COMPLIANCE_UNSPECIFIED:
            return "NaoEspecificado";
        case EXT_SPEC_COMPLIANCE_100G_AOC_OR_25GAUI_C2M_AOC_BER_5E_5:
            return "100G AOC ou 25GAUI C2M AOC (BER 5e-5)";
        case EXT_SPEC_COMPLIANCE_100GBASE_SR4_OR_25GBASE_SR:
            return "100GBASE-SR4 ou 25GBASE-SR";
        case EXT_SPEC_COMPLIANCE_100GBASE_LR4_OR_25GBASE_LR:
            return "100GBASE-LR4 ou 25GBASE-LR";
        case EXT_SPEC_COMPLIANCE_100GBASE_ER4_OR_25GBASE_ER:
            return "100GBASE-ER4 ou 25GBASE-ER";
        case EXT_SPEC_COMPLIANCE_100GBASE_SR10:
            return "100GBASE-SR10";
        case EXT_SPEC_COMPLIANCE_100G_CWDM4:
            return "100G CWDM4";
        case EXT_SPEC_COMPLIANCE_100G_PSM4:
            return "100G PSM4";
        case EXT_SPEC_COMPLIANCE_100G_ACC_OR_25GAUI_C2M_ACC_BER_5E_5:
            return "100G ACC ou 25GAUI C2M ACC (BER 5e-5)";
        case EXT_SPEC_COMPLIANCE_100GBASE_CR4_OR_25GBASE_CR_CA_25G_L_OR_50GBASE_CR2_RS_FEC:
            return "100GBASE-CR4, 25GBASE-CR CA-25G-L ou 50GBASE-CR2 RS-FEC";
        case EXT_SPEC_COMPLIANCE_25GBASE_CR_CA_25G_S_OR_50GBASE_CR2_BASE_R_FEC:
            return "25GBASE-CR CA-25G-S ou 50GBASE-CR2 BASE-R FEC";
        case EXT_SPEC_COMPLIANCE_25GBASE_CR_CA_25G_N_OR_50GBASE_CR2_NO_FEC:
            return "25GBASE-CR CA-25G-N ou 50GBASE-CR2 NO-FEC";
        case EXT_SPEC_COMPLIANCE_10MB_SINGLE_PAIR_ETHERNET:
            return "10Mb Single Pair Ethernet";
        case EXT_SPEC_COMPLIANCE_40GBASE_ER4:
            return "40GBASE-ER4";
        case EXT_SPEC_COMPLIANCE_4X_10GBASE_SR:
            return "4x10GBASE-SR";
        case EXT_SPEC_COMPLIANCE_40G_PSM4:
            return "40G PSM4";
        case EXT_SPEC_COMPLIANCE_10GBASE_T_SFI:
            return "10GBASE-T SFI";
        case EXT_SPEC_COMPLIANCE_100G_CLR4:
            return "100G CLR4";
        case EXT_SPEC_COMPLIANCE_100G_AOC_OR_25GAUI_C2M_AOC_BER_1E_12:
            return "100G AOC ou 25GAUI C2M AOC (BER 1e-12)";
        case EXT_SPEC_COMPLIANCE_100G_ACC_OR_25GAUI_C2M_ACC_BER_1E_12:
            return "100G ACC ou 25GAUI C2M ACC (BER 1e-12)";
        case EXT_SPEC_COMPLIANCE_10GBASE_T_SHORT_REACH:
            return "10GBASE-T Short Reach";
        case EXT_SPEC_COMPLIANCE_5GBASE_T:
            return "5GBASE-T";
        case EXT_SPEC_COMPLIANCE_2_5GBASE_T:
            return "2.5GBASE-T";
        case EXT_SPEC_COMPLIANCE_40G_SWDM4:
            return "40G SWDM4";
        case EXT_SPEC_COMPLIANCE_100G_SWDM4:
            return "100G SWDM4";
        case EXT_SPEC_COMPLIANCE_100G_PAM4_BIDI:
            return "100G PAM4 BiDi";
        case EXT_SPEC_COMPLIANCE_100GBASE_DR_CAUI4_NO_FEC:
            return "100GBASE-DR (CAUI-4 NO FEC)";
        case EXT_SPEC_COMPLIANCE_100G_FR_OR_100GBASE_FR1_CAUI4_NO_FEC:
            return "100G-FR/100GBASE-FR1 (CAUI-4 NO FEC)";
        case EXT_SPEC_COMPLIANCE_100G_LR_OR_100GBASE_LR1_CAUI4_NO_FEC:
            return "100G-LR/100GBASE-LR1 (CAUI-4 NO FEC)";
        case EXT_SPEC_COMPLIANCE_100GBASE_SR1_CAUI4_NO_FEC:
            return "100GBASE-SR1 (CAUI-4 NO FEC)";
        case EXT_SPEC_COMPLIANCE_100GBASE_SR1_OR_200GBASE_SR2_OR_400GBASE_SR4:
            return "100GBASE-SR1, 200GBASE-SR2 ou 400GBASE-SR4";
        case EXT_SPEC_COMPLIANCE_100GBASE_FR1_OR_400GBASE_DR4_2:
            return "100GBASE-FR1 ou 400GBASE-DR4";
        case EXT_SPEC_COMPLIANCE_100GBASE_LR1:
            return "100GBASE-LR1";
        case EXT_SPEC_COMPLIANCE_ACTIVE_CU_CABLE_50GAUI_100GAUI2_200GAUI4_C2M_BER_1E_6:
            return "Active Copper Cable (50GAUI/100GAUI-2/200GAUI-4 C2M BER 1e-6)";
        case EXT_SPEC_COMPLIANCE_ACTIVE_OPTICAL_CABLE_50GAUI_100GAUI2_200GAUI4_C2M_BER_1E_6:
            return "Active Optical Cable (50GAUI/100GAUI-2/200GAUI-4 C2M BER 1e-6)";
        case EXT_SPEC_COMPLIANCE_100GBASE_CR1_OR_200GBASE_CR2_OR_400GBASE_CR4:
            return "100GBASE-CR1, 200GBASE-CR2 ou 400GBASE-CR4";
        case EXT_SPEC_COMPLIANCE_50GBASE_CR_OR_100GBASE_CR2_OR_200GBASE_CR4:
            return "50GBASE-CR, 100GBASE-CR2 ou 200GBASE-CR4";
        case EXT_SPEC_COMPLIANCE_50GBASE_R_OR_100GBASE_SR2_OR_200GBASE_SR4:
            return "50GBASE-R, 100GBASE-SR2 ou 200GBASE-SR4";
        case EXT_SPEC_COMPLIANCE_50GBASE_FR_OR_200GBASE_DR4:
            return "50GBASE-FR ou 200GBASE-DR4";
        case EXT_SPEC_COMPLIANCE_200GBASE_FR4:
            return "200GBASE-FR4";
        case EXT_SPEC_COMPLIANCE_50GBASE_LR:
            return "50GBASE-LR";
        case EXT_SPEC_COMPLIANCE_200GBASE_LR4:
            return "200GBASE-LR4";
        case EXT_SPEC_COMPLIANCE_400GBASE_DR4_400GAUI4_C2M:
            return "400GBASE-DR4 (400GAUI-4 C2M)";
        case EXT_SPEC_COMPLIANCE_400GBASE_FR4:
            return "400GBASE-FR4";
        case EXT_SPEC_COMPLIANCE_400GBASE_LR4_6:
            return "400GBASE-LR4-6";
        case EXT_SPEC_COMPLIANCE_400G_LR4_10:
            return "400G LR4-10";
        case EXT_SPEC_COMPLIANCE_256GFC_SW4:
            return "256GFC SW4";
        case EXT_SPEC_COMPLIANCE_64GFC:
            return "64GFC";
        case EXT_SPEC_COMPLIANCE_128GFC:
            return "128GFC";
        case EXT_SPEC_COMPLIANCE_VENDOR_SPECIFIC:
            return "Específico do fabricante";
        default:
            if (code >= 0x4D && code <= 0x7E) {
                return "Reservado (SFF-8024)";
            } else if (code >= 0x82 && code <= 0xFE) {
                return "Reservado (SFF-8024)";
            }
            return "Código desconhecido";
    }
}

const char* sfp_identifier_to_string(sfp_identifier_t id) {
    switch (id) {
        case SFP_ID_GBIC:
            return "GBIC";
        case SFP_ID_SFP:
            return "SFP/SFP+";
        case SFP_ID_QSFP:
            return "QSFP";
        case SFP_ID_QSFP_PLUS:
            return "QSFP+";
        case SFP_ID_QSFP28:
            return "QSFP28";
        case SFP_ID_UNKNOWN:
        default:
            return "Desconhecido";
    }
}

const char* sfp_compliance_byte3_to_string(const sfp_compliance_decoded_t *c) {
    if (!c) return "Estrutura inválida";
    
    static char buffer[512];
    buffer[0] = '\0';
    
    if (c->eth_10g_base_er)  strcat(buffer, "  - 10GBASE-ER\n");
    if (c->eth_10g_base_lrm) strcat(buffer, "  - 10GBASE-LRM\n");
    if (c->eth_10g_base_lr)  strcat(buffer, "  - 10GBASE-LR\n");
    if (c->eth_10g_base_sr)  strcat(buffer, "  - 10GBASE-SR\n");
    
    if (c->infiniband_1x_sx)             strcat(buffer, "  - InfiniBand 1X SX\n");
    if (c->infiniband_1x_lx)             strcat(buffer, "  - InfiniBand 1X LX\n");
    if (c->infiniband_1x_copper_active)  strcat(buffer, "  - InfiniBand 1X Copper Active\n");
    if (c->infiniband_1x_copper_passive) strcat(buffer, "  - InfiniBand 1X Copper Passive\n");
    
    if (buffer[0] == '\0') return "Nenhum código ativo";
    return buffer;
}

const char* sfp_compliance_byte4_to_string(const sfp_compliance_decoded_t *c) {
    if (!c) return "Estrutura inválida";
    
    static char buffer[512];
    buffer[0] = '\0';
    
    if (c->escon_mmf)   strcat(buffer, "  - ESCON MMF\n");
    if (c->escon_smf)   strcat(buffer, "  - ESCON SMF\n");
    if (c->oc_192_sr)   strcat(buffer, "  - OC-192 SR\n");
    if (c->sonet_rs_1)  strcat(buffer, "  - SONET RS-1\n");
    if (c->sonet_rs_2)  strcat(buffer, "  - SONET RS-2\n");
    if (c->oc_48_lr)    strcat(buffer, "  - OC-48 LR\n");
    if (c->oc_48_ir)    strcat(buffer, "  - OC-48 IR\n");
    if (c->oc_48_sr)    strcat(buffer, "  - OC-48 SR\n");
    
    if (buffer[0] == '\0') return "Nenhum código ativo";
    return buffer;
}

const char* sfp_compliance_byte5_to_string(const sfp_compliance_decoded_t *c) {
    if (!c) return "Estrutura inválida";
    
    static char buffer[512];
    buffer[0] = '\0';
    
    if (c->oc_12_sm_lr) strcat(buffer, "  - OC-12 SM LR\n");
    if (c->oc_12_sm_ir) strcat(buffer, "  - OC-12 SM IR\n");
    if (c->oc_12_sr)    strcat(buffer, "  - OC-12 SR\n");
    if (c->oc_3_sm_lr)  strcat(buffer, "  - OC-3 SM LR\n");
    if (c->oc_3_sm_ir)  strcat(buffer, "  - OC-3 SM IR\n");
    if (c->oc_3_sr)     strcat(buffer, "  - OC-3 SR\n");
    
    if (buffer[0] == '\0') return "Nenhum código ativo";
    return buffer;
}

const char* sfp_compliance_byte6_to_string(const sfp_compliance_decoded_t *c) {
    if (!c) return "Estrutura inválida";
    
    static char buffer[512];
    buffer[0] = '\0';
    
    if (c->eth_base_px)      strcat(buffer, "  - BASE-PX\n");
    if (c->eth_base_bx_10)   strcat(buffer, "  - BASE-BX10\n");
    if (c->eth_100_base_fx)  strcat(buffer, "  - 100BASE-FX\n");
    if (c->eth_100_base_lx)  strcat(buffer, "  - 100BASE-LX\n");
    if (c->eth_1000_base_t)  strcat(buffer, "  - 1000BASE-T\n");
    if (c->eth_1000_base_cx) strcat(buffer, "  - 1000BASE-CX\n");
    if (c->eth_1000_base_lx) strcat(buffer, "  - 1000BASE-LX\n");
    if (c->eth_1000_base_sx) strcat(buffer, "  - 1000BASE-SX\n");
    
    if (buffer[0] == '\0') return "Nenhum código ativo";
    return buffer;
}

const char* sfp_compliance_byte7_to_string(const sfp_compliance_decoded_t *c) {
    if (!c) return "Estrutura inválida";
    
    static char buffer[512];
    buffer[0] = '\0';
    
    if (c->fc_very_long_distance)      strcat(buffer, "  - Very Long Distance\n");
    if (c->fc_short_distance)          strcat(buffer, "  - Short Distance\n");
    if (c->fc_intermediate_distance)   strcat(buffer, "  - Intermediate Distance\n");
    if (c->fc_long_distance)           strcat(buffer, "  - Long Distance\n");
    if (c->fc_medium_distance)         strcat(buffer, "  - Medium Distance\n");
    if (c->shortwave_laser_sa)         strcat(buffer, "  - Shortwave Laser (SA)\n");
    if (c->longwave_laser_lc)          strcat(buffer, "  - Longwave Laser (LC)\n");
    if (c->electrical_inter_enclosure) strcat(buffer, "  - Electrical Inter-Enclosure\n");
    
    if (buffer[0] == '\0') return "Nenhum código ativo";
    return buffer;
}

const char* sfp_compliance_byte8_to_string(const sfp_compliance_decoded_t *c) {
    if (!c) return "Estrutura inválida";
    
    static char buffer[512];
    buffer[0] = '\0';
    
    if (c->electrical_intra_enclosure) strcat(buffer, "  - Electrical Intra-Enclosure\n");
    if (c->shortwave_laser_sn)         strcat(buffer, "  - Shortwave Laser (SN)\n");
    if (c->shortwave_laser_sl)         strcat(buffer, "  - Shortwave Laser (SL)\n");
    if (c->longwave_laser_ll)          strcat(buffer, "  - Longwave Laser (LL)\n");
    if (c->active_cable)               strcat(buffer, "  - Active SFP+ Cable\n");
    if (c->passive_cable)              strcat(buffer, "  - Passive SFP+ Cable\n");
    
    if (buffer[0] == '\0') return "Nenhum código ativo";
    return buffer;
}

const char* sfp_compliance_byte9_to_string(const sfp_compliance_decoded_t *c) {
    if (!c) return "Estrutura inválida";
    
    static char buffer[512];
    buffer[0] = '\0';
    
    if (c->twin_axial_pair) strcat(buffer, "  - Twin Axial Pair\n");
    if (c->twisted_pair)    strcat(buffer, "  - Twisted Pair\n");
    if (c->miniature_coax)  strcat(buffer, "  - Miniature Coax\n");
    if (c->video_coax)      strcat(buffer, "  - Video Coax\n");
    if (c->multimode_m6)    strcat(buffer, "  - Multimode 62.5 µm\n");
    if (c->multimode_m5)    strcat(buffer, "  - Multimode 50 µm\n");
    if (c->single_mode)     strcat(buffer, "  - Single Mode\n");
    
    if (buffer[0] == '\0') return "Nenhum código ativo";
    return buffer;
}

const char* sfp_compliance_byte10_to_string(const sfp_compliance_decoded_t *c) {
    if (!c) return "Estrutura inválida";
    
    static char buffer[512];
    buffer[0] = '\0';
    
    if (c->cs_1200_mbps) strcat(buffer, "  - 1200 Mbps\n");
    if (c->cs_800_mbps)  strcat(buffer, "  - 800 Mbps\n");
    if (c->cs_1600_mbps) strcat(buffer, "  - 1600 Mbps\n");
    if (c->cs_400_mbps)  strcat(buffer, "  - 400 Mbps\n");
    if (c->cs_3200_mbps) strcat(buffer, "  - 3200 Mbps\n");
    if (c->cs_200_mbps)  strcat(buffer, "  - 200 Mbps\n");
    if (c->cs_100_mbps)  strcat(buffer, "  - 100 Mbps\n");
    
    if (buffer[0] == '\0') return "Nenhum código ativo";
    return buffer;
}

const char* sfp_encoding_to_string(sfp_encoding_codes_t encoding) {
    static char unknown_buffer[50];
    switch ((uint8_t)encoding) {
        case SFP_ENC_UNSPECIFIED:
            return "Unspecified";
        case SFP_ENC_8B_10B:
            return "8B/10B";
        case SFP_ENC_4B_5B:
            return "4B/5B";
        case SFP_ENC_NRZ:
            return "NRZ";
        case SFP_ENC_MANCHESTER:
            return "Manchester";
        case SFP_ENC_SONET_SCRAMBLED:
            return "SONET Scrambled";
        case SFP_ENC_64B_66B:
            return "64B/66B";
        case SFP_ENC_256B_257B:
            return "256B/257B";
        case SFP_ENC_PAM4:
            return "PAM4";
        default:
            // Para códigos reservados/desconhecidos, não podemos retornar uma string estática porque precisamos do valor.
            // Vamos usar um buffer estático para formatar a string.
            snprintf(unknown_buffer, sizeof(unknown_buffer), "Reserved(0x%02X)", (uint8_t)encoding);
            return unknown_buffer;
    }
}

const char* sfp_om2_to_string(sfp_om2_length_status_t om2_status, uint16_t om2_length_m) {
    static char buffer[100];  // Buffer estático para armazenar a string
    
    switch (om2_status) {
        case SFP_OM2_LEN_VALID:
            snprintf(buffer, sizeof(buffer), "%u metros", om2_length_m);
            return buffer;
            
        case SFP_OM2_LEN_EXTENDED:
            snprintf(buffer, sizeof(buffer), "Extendido");
            return buffer;
            
        case SFP_OM2_LEN_NOT_SUPPORTED:
        default:
            return "Nao Suportado";
    }
}

