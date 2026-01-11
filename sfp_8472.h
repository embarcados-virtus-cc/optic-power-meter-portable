/**
 * @file sfp_8472.h
 * @brief Biblioteca de parsing SFF-8472 – Base ID (A0h)
 *
 * @author Alexandre Junior
 * @date 2026-01-10
 *
 * @details
 *  Implementa a leitura e interpretação dos campos da EEPROM A0h
 *  de módulos SFP/SFP+, conforme a especificação SFF-8472.
 *
 * @note
 *  Este arquivo define apenas a API pública e estruturas de dados.
 */


/**
 * SFF-8472 Management Interface Library - Data Structures
 * Rev 12.5 (August 1, 2025) compatible
 */



#ifndef SFF_8472_H
#define SFF_8472_H

#include <stdint.h>
#include <stdbool.h>


#include "hardware/i2c.h"
#include "pico/types.h"



/**********************************************
 * Basic Type Definitions
 **********************************************/

#define SFP_I2C_ADDR_A0   0x50
#define SFP_I2C_ADDR_A2   0x51


#define SFP_A0_BASE_SIZE  64
#define SFP_A2_SIZE       256


/*==========================================
 * Byte 0 — Identifier (SFF-8472 / SFF-8024)
 ===========================================*/

typedef enum {
    SFP_ID_UNKNOWN   = 0x00,
    SFP_ID_GBIC      = 0x02,
    SFP_ID_SFP       = 0x03,
    SFP_ID_QSFP      = 0x0C,
    SFP_ID_QSFP_PLUS = 0x11,
    SFP_ID_QSFP28    = 0x18
} sfp_identifier_t;


/*
 * Status da informação de alcance OM1
 */
typedef enum {
    SFP_OM1_LEN_NOT_SUPPORTED,   /* Byte 17 = 0x00 */
    SFP_OM1_LEN_VALID,           /* 0x01–0xFE */
    SFP_OM1_LEN_EXTENDED         /* Byte 17 = 0xFF */
} sfp_om1_length_status_t;

/*
 * Status da informação de alcance OM4
 */

typedef enum {
  SFP_OM4_LEN_NOT_SUPPORTED,    /* Byte 18 = 0x00 */
  SFP_OM4_LEN_VALID,            /* Byte 18 = 0x01–0xFE */
  SFP_OM4_LEN_EXTENDED          /* Byte 18 = 0xFF */
} sfp_om4_length_status_t;


/**********************************************
 * A0h Memory Map (128 bytes) - Base ID Fields
 **********************************************/

typedef struct {
    /* Byte 0: Identifier */
     sfp_identifier_t identifier;           // Table 5-1

    /* Byte 1: Extended Identifier */
    uint8_t ext_identifier;       // Table 5-2

    /* Byte 2: Connector */
    uint8_t connector;            // SFF-8024

    /* Bytes 3-10: Transceiver Compliance Codes */
    struct {
        uint8_t byte3;   // Ethernet/InfiniBand/ESON bits
        uint8_t byte4;   // SONET/SFP+ Cable Tech/Fibre Channel Media
        uint8_t byte5;   // Fibre Channel Media/Speed
        uint8_t byte6;   // Ethernet Compliance
        uint8_t byte7;   // Fibre Channel Link Length
        uint8_t byte8;   // Fibre Channel Technology
        uint8_t byte9;   // Fibre Channel Speed
        uint8_t byte10;  // Fibre Channel Speed 2
    } compliance_codes;

    /* Byte 11: Encoding */
    uint8_t encoding;             // SFF-8024

    /* Byte 12: Signaling Rate, Nominal */
    uint8_t nominal_rate;         // Units of 100 MBd

    /* Byte 13: Rate Identifier */
    uint8_t rate_identifier;      // Table 5-6


    /*Byte 17: 62.5 um OM1*/
    uint16_t om1_length_m;           /* Distância convertida (metros) */
    sfp_om1_length_status_t om1_status;

    uint16_t om4_or_cooper_length_m;
    sfp_om4_length_status_t om4_or_cooper_status;

    /* Bytes 14-19: Link Lengths (MODIFICAR,ESTA ESTRUTURA NÃO É APROPRIADA PARA ESSA DADOS) */

   /* struct {
        uint8_t smf_km;           // Single mode km or copper attenuation @12.9 GHz
        uint8_t smf_100m;         // Single mode 100m or copper attenuation @25.78 GHz
        uint8_t om2_10m;          // 50um OM2 in 10m units
        uint8_t om1_10m;          // 62.5um OM1 in 10m units
        uint8_t om4_or_copper;    // OM4 in 10m or copper in meters
        uint8_t om3_or_cable_add; // OM3 in 10m or cable multiplier+base
    } length;
    */

    /* Bytes 20-35: Vendor Name (ASCII) */
    char vendor_name[16];

    /* Byte 36: Extended Compliance Codes */
    uint8_t ext_compliance;       // SFF-8024 Table 4-4

    /* Bytes 37-39: Vendor OUI */
    uint8_t vendor_oui[3];

    /* Bytes 40-55: Vendor Part Number (ASCII) */
    char vendor_pn[16];

    /* Bytes 56-59: Vendor Revision (ASCII) */
    char vendor_rev[4];

    /* Bytes 60-61: Wavelength or Cable Compliance */
    union {
        uint16_t wavelength;      // Optical: wavelength in nm
        struct {
            uint8_t passive_bits; // Table 8-1
            uint8_t active_bits;  // Table 8-2
        } cable_compliance;
    } media_info;

    /* Byte 62: Fibre Channel Speed 2 */
    uint8_t fc_speed2;

    /* Byte 63: CC_BASE (Checksum) */
    uint8_t cc_base;
} sfp_a0h_base_t;
/**********************************************
 * Function Prototypes for Library
 **********************************************/

/* Initialization and Discovery */
bool sff_module_init(); /* MELHORAR A IMPLEMENTAÇÃO, PARA EVITAR O USO DE VARIAS FUNÇÕES DE INICIALIZAÇÃO */
bool sfp_i2c_init(i2c_inst_t *i2c, uint sda, uint scl, uint baudrate);

/* Memory Access */
bool sfp_read_block(i2c_inst_t *i2c,uint8_t dev_addr,uint8_t start_offset,uint8_t *buffer,uint8_t length);



/* Functions */

void sfp_parse_a0_base_identifier(const uint8_t *a0_base_data,sfp_a0h_base_t *a0);
sfp_identifier_t sfp_a0_get_identifier(const sfp_a0h_base_t *a0);

/* Byte 17 OM1 (62.5 um) */
void sfp_parse_a0_base_om1(const uint8_t *a0_base_data,sfp_a0h_base_t *a0);
uint16_t sfp_a0_get_om1_length_m(const sfp_a0h_base_t *a0,sfp_om1_length_status_t *status);

/* Byte 18 OM4 or copper cable */
void sfp_parse_a0_base_om4_or_copper(const uint8_t *a0_base_data, sfp_a0h_base_t *a0);
uint16_t sfp_a0_get_om4_copper_or_length_m(const sfp_a0h_base_t *a0, sfp_om4_length_status_t *status);


#endif /* SFF_8472_H */
