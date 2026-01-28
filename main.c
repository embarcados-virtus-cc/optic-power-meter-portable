/**
 * @file main.c
 * @brief Ponto de entrada principal do sistema SFP/SFP+
 * 
 * Este arquivo contém a função main que inicializa e executa o loop principal
 * do sistema de menu SFP/SFP+ com controle por joystick.
 */

#include <stdio.h>
#include "pico/stdlib.h"

#include "sfp_8472/sfp_8472.h"
#include "menu/menu.h"


/* Barramento físico */
#define I2C_PORT i2c0
#define I2C_SDA  0
#define I2C_SCL  1

/**
 * @brief Ponto de entrada principal do programa
 * 
 * Inicializa o sistema, carrega dados do SFP e executa o loop principal
 * de processamento de entrada, atualização de dados e renderização.
 * 
 * @return int Código de retorno do programa (0 para sucesso)
 */
int main(void) {
    // Inicialização do sistema
    stdio_init_all();
    ssd1306_Init();
    joystickPi_init();
    
    
    /* Inicializa I2C */
    sfp_i2c_init(
        I2C_PORT,
        I2C_SDA,
        I2C_SCL,
        100 * 1000
    );
    
    /* Buffer cru(raw) da EEPROM A0h */
    uint8_t a0_base_data[SFP_A0_BASE_SIZE] = {0};
    
     bool ok = sfp_read_block(
        I2C_PORT,
        SFP_I2C_ADDR_A0,
        0x00,
        a0_base_data,
        SFP_A0_BASE_SIZE
    );
    
     if (!ok) {
        //printf("ERRO: Falha na leitura do A0h\n");
        while (1);
    }
    
    // Inicialização de dados do SFP
    init_sfp_data();
    
    /*Colocar Init()*/
    sfp_parse_a0_base_identifier(a0_base_data, &system_ctrl.a0);
    
    /*Byte 3 - 10*/
    sfp_read_compliance(a0_base_data,&system_ctrl.a0.cc);
    sfp_decode_compliance(&system_ctrl.a0.cc,&system_ctrl.a0.dc);
    
    /*Byte 11*/
    sfp_parse_a0_base_encoding(a0_base_data,&system_ctrl.a0);
    
    /*Byte 12*/
    sfp_parse_a0_base_nominal_rate(a0_base_data, &a0);
    /*Byte 16*/
    sfp_parse_a0_base_om2(a0_base_data,&system_ctrl.a0);
    
    /*Byte 17*/
    
    /*Byte 20-35*/
    sfp_parse_a0_base_vendor_name(a0_base_data, &a0);
    
    /*Byte 36*/
    sfp_parse_a0_base_ext_compliance(a0_base_data, &system_ctrl.a0);
    
    

    
    // Configuração inicial dos timers
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Inicializa a estrutura de controle do sistema
    // Nota: A estrutura system_ctrl é estática em menu.c, então
    // precisamos inicializar através das funções do menu
    
    // Mensagem de boas-vindas
    ssd1306_Fill(Black);
    ssd1306_SetCursor(20, 20);
    ssd1306_WriteString("SFP/SFP+ SYSTEM", Font_7x10, White);
    ssd1306_SetCursor(35, 40);
    ssd1306_WriteString("v1.2", Font_6x8, White);
    ssd1306_UpdateScreen();
    sleep_ms(2000);
    
    // Loop principal
    while (true) {
        // Processa entrada do joystick
        process_joystick_input();
        
        // Atualiza dados do sistema
        update_system_data();
        
        // Renderiza tela atual
        render_current_screen();
        
        // Atualiza display
        ssd1306_UpdateScreen();
        
        // Pequena pausa para controle de atualização
        sleep_ms(50);
    }
    
    return 0;
}
