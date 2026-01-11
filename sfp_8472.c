#include "sfp_8472.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <cstdint>



bool sfp_i2c_init(i2c_inst_t *i2c, uint sda, uint scl, uint baudrate)
{
    i2c_init(i2c, baudrate);

    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);

    gpio_pull_up(sda);
    gpio_pull_up(scl);

    return true;
}


bool sfp_read_block(i2c_inst_t *i2c,uint8_t dev_addr,uint8_t start_offset,uint8_t *buffer,uint8_t length)
{
    if (!buffer || length == 0)
        return false;

    /* 1. Envia offset interno */
    int ret = i2c_write_blocking(
        i2c,
        dev_addr,
        &start_offset,
        1,
        true               // repeated start
    );

    if (ret != 1)
        return false;

    /* 2. Lê dados sequenciais (EEPROM)*/
    ret = i2c_read_blocking(
        i2c,
        dev_addr,
        buffer,
        length,
        false
    );

    return (ret == length);
}

/*
 * Faz o parsing do bloco A0h.
 * Apenas o Byte 0 é tratado.
 */
void sfp_parse_a0_base_identifier(const uint8_t *a0_base_data,sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;

    /*
     * Byte 0 — Identifier
     * Copiado diretamente da EEPROM para a struct.
     */
    a0->identifier = (sfp_identifier_t)a0_base_data[0];
}

/*
 * Getter simples.
 */
sfp_identifier_t sfp_a0_get_identifier(const sfp_a0h_base_t *a0)
{
    if (!a0)
        return SFP_ID_UNKNOWN;

    return a0->identifier;
}


void sfp_parse_a0_base_om1(const uint8_t *a0_base_data,sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;

    /* =========================================================
     * Byte 17 — Length (OM1, 62.5 µm)
     * =========================================================*/
    uint8_t raw = a0_base_data[17];

    /*
     * Fluxo Principal + Secundários
     */
    if (raw == 0x00) {
        /*
         * Fluxo Secundário 2 (caso 00h):
         * Não há informação explícita de alcance OM1.
         * Inferência futura via bytes 3–10.
         */
        a0->om1_status   = SFP_OM1_LEN_NOT_SUPPORTED;
        a0->om1_length_m = 0;
    }
    else if (raw == 0xFF) {
        /*
         * Fluxo Secundário 2 (caso FFh):
         * Alcance superior ao máximo nominal do campo.
         * Representa > 2,54 km.
         */
        a0->om1_status   = SFP_OM1_LEN_EXTENDED;
        a0->om1_length_m = 2540; /* Limite inferior conhecido */
    }
    else {
        /*
         * Fluxo Principal / Secundário 1:
         * Valor válido (01h–FEh)
         * Unidade: 10 metros
         */
        a0->om1_status   = SFP_OM1_LEN_VALID;
        a0->om1_length_m = (uint16_t)raw * 10;
    }
}

/*
 * Getter simples.
 */

uint16_t sfp_a0_get_om1_length_m(const sfp_a0h_base_t *a0,sfp_om1_length_status_t *status)
{
    if (!a0) {
        if (status)
            *status = SFP_OM1_LEN_NOT_SUPPORTED;
        return 0;
    }

    if (status)
        *status = a0->om1_status;

    return a0->om1_length_m;
}


/* Função auxiliar para saber se é um cabo de cobre ou não */
static bool sfp_is_copper(uint8_t byte8)
{
    /* Se é um cabo de cobre, o bit 2 ou 3 está ativo. */
    return (byte8 & (1 << 2)) || (byte8 & (1 << 3));
}

void sfp_parse_a0_base_om4_or_copper(const uint8_t *a0_base_data, sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;

    /* =========================================================
     * Byte 18 — Length (OM4 or copper cable)
     * =========================================================*/
    uint8_t raw_length = a0_base_data[18];
    /* =========================================================
     * Byte 8 — Natureza física do meio
     * =========================================================*/
    uint8_t byte8 = a0_base_data[8];

    bool is_copper = sfp_is_copper(byte8);

    /*
     * Fluxo Principal + Secundários
     */
    if (raw_length == 0x00) {
        /*
         * Fluxo Secundário 2 (caso 00h):
         * Não há informação explícita de comprimento de OM4 ou o cobre sem informação válida.
         */
        a0->om4_or_cooper_status = SFP_OM4_LEN_NOT_SUPPORTED;
        a0->om4_or_cooper_length_m = 0;
    }
    else if (raw_length == 0xFF) {
        /*
         * Fluxo Secundário (caso FFh):
         * Comprimento superior ao máximo nominal representável
         * pelo campo.
         *
         * - Cabo de cobre: comprimento > 254 m
         * - OM4: comprimento > 2,54 km
         *
         * O valor armazenado representa o limite inferior conhecido.
         */
        a0->om4_or_cooper_status = SFP_OM4_LEN_EXTENDED;

        if (is_copper)
            a0->om4_or_cooper_length_m = 254;
        else
            a0->om4_or_cooper_length_m = 2540;
    }
    else {
        /*
         * Fluxo Principal:
         * Valor válido (01h–FEh)
         *
         * - OM4: unidades de 10 metros
         * - Cabo de cobre: unidades de 1 metro
         */
        a0->om4_or_cooper_status = SFP_OM4_LEN_VALID;

        if (is_copper)
            a0->om4_or_cooper_length_m = raw_length;
        else
            a0->om4_or_cooper_length_m = (uint16_t)raw_length * 10;
    }
}

/*
 * Getter simples.
 */

uint16_t sfp_a0_get_om4_copper_or_length_m(const sfp_a0h_base_t *a0, sfp_om4_length_status_t *status)
{
    if (!a0) {
        if (!status) {
            *status = SFP_OM4_LEN_NOT_SUPPORTED;
        }
        return 0;
    }

    if (status)
        *status = a0->om4_or_cooper_status;

    return a0->om4_or_cooper_length_m;
}
