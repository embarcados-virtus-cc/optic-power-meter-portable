#include <stdio.h>
#include "pico/stdlib.h"

#include "sfp_8472.h"

/* Barramento físico */
#define I2C_PORT i2c0
#define I2C_SDA  0
#define I2C_SCL  1

int main(void)
{
    /* Inicialização da UART USB */
    stdio_init_all();

    /*Tempo para concetar ao um Leitor Serial*/
    sleep_ms(2000);

    printf("=== Teste SFP A0h (Byte 0 e Byte 17) ===\n");

    /* Inicializa I2C */
    sfp_i2c_init(
        I2C_PORT,
        I2C_SDA,
        I2C_SCL,
        100 * 1000
    );

    /* Buffer cru(raw) da EEPROM A0h */
    uint8_t a0_base_data[SFP_A0_BASE_SIZE] = {0};

    printf("Lendo EEPROM A0h...\n");

    bool ok = sfp_read_block(
        I2C_PORT,
        SFP_I2C_ADDR_A0,
        0x00,
        a0_base_data,
        SFP_A0_BASE_SIZE
    );

    if (!ok) {
        printf("ERRO: Falha na leitura do A0h\n");
        while (1);
    }

    printf("Leitura A0h OK\n");

    /* Estrutura interpretada */
    sfp_a0h_base_t a0 = {0};

    /* Parsing do bloco */
    sfp_parse_a0_base_identifier(a0_base_data, &a0);

    /* =====================================================
     * Teste do Byte 0 — Identifier
     * ===================================================== */
    sfp_identifier_t id = sfp_a0_get_identifier(&a0);

    printf("\nByte 0 — Identifier: 0x%02X\n", id);

    if (id == SFP_ID_SFP) {
        printf("Modulo SFP/SFP+ identificado corretamente\n");
    } else {
        printf("Modulo nao suportado ou invalido\n");
    }

    /* =====================================================
     * Teste do Byte 17 — Length OM1 (62.5 µm)
     * ===================================================== */
    sfp_parse_a0_base_om1(a0_base_data,&a0);
    sfp_om1_length_status_t om1_status;
    uint16_t om1_length_m = sfp_a0_get_om1_length_m(&a0, &om1_status);

    printf("\nByte 17 — Length OM1 (62.5 µm)\n");

    switch (om1_status) {

    case SFP_OM1_LEN_VALID:
        printf("Alcance OM1 valido: %u metros\n", om1_length_m);
        break;

    case SFP_OM1_LEN_EXTENDED:
        printf("Alcance OM1 superior a %u metros (>2.54 km)\n",
               om1_length_m);
        break;

    case SFP_OM1_LEN_NOT_SUPPORTED:
    default:
        printf("Alcance OM1 nao especificado ou nao suportado\n");
        break;
    }

    /* =====================================================
     * Teste do Byte 18 — Length OM4 or copper cable
     * ===================================================== */
     sfp_parse_a0_base_om4_or_copper(a0_base_data, &a0);

     sfp_om4_length_status_t om4_status;

     uint16_t om4_length_m = sfp_a0_get_om4_copper_or_length_m(&a0, &om4_status);

     printf("\nByte 18 — Length OM4 or Copper Cable\n");

     switch (om4_status) {

     case SFP_OM4_LEN_VALID:
         printf("Comprimento valido: %u metros\n", om4_length_m);
         break;

     case SFP_OM1_LEN_EXTENDED:
         printf("Comprimento superior a %u metros\n", om4_length_m);
         break;

     case SFP_OM1_LEN_NOT_SUPPORTED:
     default:
         printf("Comprimento nao especificado\n");
         break;
     }

#ifdef DEBUG
    /* Dump (opcional) para inspeção manual */
    printf("\nDump EEPROM A0h:");
    for (int i = 0; i < SFP_A0_BASE_SIZE; i++) {
        if (i % 16 == 0)
            printf("\n%02X: ", i);
        printf("%02X ", a0_base_data[i]);
    }
    printf("\n");
#endif

    printf("\nTeste concluido. Sistema em idle.\n");

    while (1) {
        sleep_ms(1000);
    }
}
