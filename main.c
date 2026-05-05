/**
 * @file main.c
 * @brief Ponto de entrada principal do sistema SFP/SFP+
 * 
 * Este arquivo contém a função main que inicializa e executa o loop principal
 * do sistema de menu SFP/SFP+ com controle por joystick.
 */

#include <stdio.h>
#include "pico/stdlib.h"

#include "I2C/i2c.h"
#include "sfp_8472/a0h.h"
#include "sfp_8472/a2h.h"
#include "sfp_8472/sfp.h"
#include "menu/menu.h"


/* Barramento físico */
#define I2C_PORT i2c1
#define I2C_SDA  11
#define I2C_SCL  10

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

    sleep_ms(2000);/*Delay para inicializar os dados corretamente*/
    /*sfp_t sfp;*/
    
    
    /* Inicializa I2C */
    sfp_i2c_init(
        I2C_PORT,
        I2C_SDA,
        I2C_SCL,
        100 * 1000
    );

    // Inicialização de dados do SFP
    init_sfp_data();
    
    // Mensagem de boas-vindas
    ssd1306_Fill(Black);
    ssd1306_SetCursor(20, 20);
    ssd1306_WriteString("SFP/SFP+ SYSTEM", Font_7x10, White);
    ssd1306_SetCursor(35, 40);
    ssd1306_WriteString("v1.2", Font_6x8, White);
    ssd1306_UpdateScreen();
    sleep_ms(2000);

    //init_sfp_data();
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
