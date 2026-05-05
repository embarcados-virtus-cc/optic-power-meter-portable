#include "sfp.h"
#include "I2C/i2c.h"

bool sfp_init(sfp_t *sfp){
  if(!sfp){
    return false;
  }

  if(!sfp_update(sfp)){
    return false;
  }

  return true;
}

bool sfp_update(sfp_t *sfp){
  if(!sfp){
    return false;
  }

  if (!sfp_update_a0(sfp)) {
    return false;
  }

  if(!sfp_update_a2(sfp)){
    return false;
  }

  return true;
  
}

bool sfp_update_a0(sfp_t *sfp){

  uint8_t a0_base_data[SFP_A0_SIZE] = {0};

  bool ok = sfp_read_block(
        I2C_PORT,
        SFP_I2C_ADDR_A0,
        0x00,
        a0_base_data,
        SFP_A0_SIZE
    );

/* ===== Bytes Básicos (0-63) ===== */

  /* Byte 0 - Identifier */
  sfp_parse_a0_base_identifier(a0_base_data, &sfp->a0);

  /* Byte 63 - CC_BASE Checksum */
  sfp_parse_a0_base_cc_base(a0_base_data, &sfp->a0);


  /*Verificação do Checksum e identificação do SFP*/
  if(sfp_a0_get_cc_base_is_valid(&sfp->a0)  && (sfp_a0_get_identifier(&sfp->a0) == SFP_ID_SFP) ){
 /* Byte 1 - Extended Identifier */
    sfp_parse_a0_base_ext_identifier(a0_base_data,  &sfp->a0);
    
    /* Byte 2 - Connector */
    sfp_parse_a0_base_connector(a0_base_data,  &sfp->a0);
    
    /* Bytes 3-10 - Compliance Codes */
    sfp_parse_a0_base_compliance(a0_base_data,  &sfp->a0.cc);
    sfp_a0_decode_compliance(&sfp->a0.cc, &sfp->a0.dc);
    
    /* Byte 11 - Encoding */
    sfp_parse_a0_base_encoding(a0_base_data,  &sfp->a0);
    
    /* Byte 12 - Nominal Rate */
    sfp_parse_a0_base_nominal_rate(a0_base_data,  &sfp->a0);
    
    /* Byte 13 - Rate Identifier */
    sfp_parse_a0_base_rate_identifier(a0_base_data,  &sfp->a0);
    
    /* Byte 14 - SMF Length (km) */
    sfp_parse_a0_base_smf_km(a0_base_data,  &sfp->a0);
    
    /* Byte 15 - SMF Length (100m) */
    sfp_parse_a0_base_smf_m(a0_base_data,  &sfp->a0);
    
    /* Byte 16 - OM2 Length */
    sfp_parse_a0_base_om2(a0_base_data,  &sfp->a0);
    
    /* Byte 17 - OM1 Length */
    sfp_parse_a0_base_om1(a0_base_data,  &sfp->a0);
    
    /* Byte 18 - OM4 or Copper */
    sfp_parse_a0_base_om4_or_copper(a0_base_data,  &sfp->a0);
    
    /* Byte 19 - OM3 or Cable */
    sfp_parse_a0_base_om3_or_cable(a0_base_data,  &sfp->a0);
    
    /* Bytes 20-35 - Vendor Name */
    sfp_parse_a0_base_vendor_name(a0_base_data,  &sfp->a0);
    
    /* Byte 36 - Extended Compliance */
    sfp_parse_a0_base_ext_compliance(a0_base_data,  &sfp->a0);
    
    /* Bytes 37-39 - Vendor OUI */
    sfp_parse_a0_base_vendor_oui(a0_base_data,  &sfp->a0);
    
    /* Bytes 40-55 - Vendor PN */
    sfp_parse_a0_base_vendor_pn(a0_base_data,  &sfp->a0);
    
    /* Bytes 56-59 - Vendor Rev */
    sfp_parse_a0_base_vendor_rev(a0_base_data,  &sfp->a0);
    
    /* Bytes 60-61 - Wavelength/Media */
    sfp_parse_a0_base_media(a0_base_data,  &sfp->a0);
    
    /* Byte 62 - FC Speed 2 */
    sfp_parse_a0_fc_speed_2(a0_base_data,  &sfp->a0);

     /* ===== Extended Fields (92+) ===== */
    /* Byte 92 - DMI, Change Address, Calibration */
    sfp_parse_a0_extended_dmi(a0_base_data, &sfp->a0e);
    sfp_parse_a0_extended_change_addr_req(a0_base_data, &sfp->a0e);
    sfp_parse_a0_extended_calibration(a0_base_data, &sfp->a0e);

    return true;


  }

  return false;
 
}

bool sfp_update_a2(sfp_t *sfp){

   /*Adicionar Condicional depois (A2H)*/
  if (true) {


    uint8_t a2_data[SFP_A2_SIZE];

    bool ok = sfp_read_block(
        I2C_PORT,
        SFP_I2C_ADDR_A2,
        0x00,
        a2_data,
        SFP_A2_SIZE
    );
  
 /* ===== Thresholds de Temperatura (Bytes 00-07) ===== */
    /* Bytes 00-01 - High Temperature Alarm */
    sfp_parse_a2h_temp_high_alarm(a2_data,&sfp->a2);
    
    /* Bytes 02-03 - Low Temperature Alarm */
    sfp_parse_a2h_temp_low_alarm(a2_data, &sfp->a2);
    
    /* Bytes 04-05 - High Temperature Warning */
    sfp_parse_a2h_temp_high_warning(a2_data,  &sfp->a2);
    
    /* Bytes 06-07 - Low Temperature Warning */
    sfp_parse_a2h_temp_low_warning(a2_data,  &sfp->a2);

    /* ===== Thresholds de Tensão (VCC) (Bytes 08-15) ===== */
    /* Bytes 08-09 - High Alarm VCC */
    sfp_parse_a2h_vcc_high_alarm(a2_data,  &sfp->a2);
    
    /* Bytes 10-11 - Low Alarm VCC */
    sfp_parse_a2h_vcc_low_alarm(a2_data,  &sfp->a2);
    
    /* Bytes 12-13 - High Warning VCC */
    sfp_parse_a2h_vcc_high_warning(a2_data,  &sfp->a2);
    
    /* Bytes 14-15 - Low Warning VCC */
    sfp_parse_a2h_vcc_low_warning(a2_data,  &sfp->a2);

    /* ===== Thresholds de Corrente de Bias (Bytes 16-23) ===== */
    /* Bytes 16-17 - High Alarm TX Bias */
    sfp_parse_a2h_tx_bias_high_alarm(a2_data,  &sfp->a2);
    
    /* Bytes 18-19 - Low Alarm TX Bias */
    sfp_parse_a2h_tx_bias_low_alarm(a2_data,  &sfp->a2);
    
    /* Bytes 20-21 - High Warning TX Bias */
    sfp_parse_a2h_tx_bias_high_warning(a2_data,  &sfp->a2);
    
    /* Bytes 22-23 - Low Warning TX Bias */
    sfp_parse_a2h_tx_bias_low_warning(a2_data,  &sfp->a2);

    /* ===== Thresholds de Potência de Transmissão (Bytes 24-31) ===== */
    /* Bytes 24-25 - High Alarm TX Power */
    sfp_parse_a2h_tx_power_high_alarm(a2_data,  &sfp->a2);
    
    /* Bytes 26-27 - Low Alarm TX Power */
    sfp_parse_a2h_tx_power_low_alarm(a2_data,  &sfp->a2);
    
    /* Bytes 28-29 - High Warning TX Power */
    sfp_parse_a2h_tx_power_high_warning(a2_data,  &sfp->a2);
    
    /* Bytes 30-31 - Low Warning TX Power */
    sfp_parse_a2h_tx_power_low_warning(a2_data,  &sfp->a2);

    /* ===== Thresholds de Potência de Recepção (Bytes 32-39) ===== */
    /* Bytes 32-33 - High Alarm RX Power */
    sfp_parse_a2h_rx_power_high_alarm(a2_data,  &sfp->a2);
    
    /* Bytes 34-35 - Low Alarm RX Power */
    sfp_parse_a2h_rx_power_low_alarm(a2_data,  &sfp->a2);
    
    /* Bytes 36-37 - High Warning RX Power */
    sfp_parse_a2h_rx_power_high_warning(a2_data,  &sfp->a2);
    
    /* Bytes 38-39 - Low Warning RX Power */
    sfp_parse_a2h_rx_power_low_warning(a2_data,  &sfp->a2);

    /*Bytes 96-97 Temperature*/
    sfp_parse_a2h_temperature(a2_data,&sfp->a2);

    /*Bytes 98-99 VCC*/
    sfp_parse_a2h_vcc(a2_data,&sfp->a2);

    /* Bytes 100-101 - TX Bias atual */
    sfp_parse_a2h_tx_bias(a2_data,  &sfp->a2);
    
    /* Bytes 102-103 - TX Power atual */
    sfp_parse_a2h_tx_power(a2_data,  &sfp->a2);
    
    /* Bytes 104-105 - RX Power atual */
    sfp_parse_a2h_rx_power(a2_data,  &sfp->a2);

    /* ===== Status e Controle ===== */
    /* Byte 110 - Data Ready (ELA DEVE FICAR NO TOPO, MAS AINDA ESTOU PENSANDO EM UM ALGORITMO DE RELEITURA CASO
     * NÃO ESTEJAM PRONTOS)*/
    sfp_parse_a2h_data_ready(a2_data,  &sfp->a2);

    return true;
  }

  return false;
}
