import matplotlib.pyplot as plt

# Dados extraídos da imagem
att_voa_db = [1, 2, 3, 4, 5, 6]
power_meter_dbm = [-10.73, -11.45, -12.36, -13.32, -14.29, -15.27]
sfp_dbm = [-10.7, -11.4, -12.4, -13.3, -14.2, -30.0]

# Criação do gráfico
plt.figure(figsize=(10, 6))

# Plotando as duas linhas
plt.plot(att_voa_db, power_meter_dbm, marker='o', linestyle='-', label='Power Meter (dBm)', color='blue')
plt.plot(att_voa_db, sfp_dbm, marker='s', linestyle='--', label='Saída SFP (dBm)', color='red')

# Formatação do gráfico
plt.title('Comparação: Power Meter vs Saída SFP')
plt.xlabel('Atenuação VOA (dB)')
plt.ylabel('Potência (dBm)')
plt.grid(True, linestyle=':', alpha=0.6)
plt.legend()

# Inverter o eixo Y pode ajudar na visualização, já que são valores negativos de potência
# plt.gca().invert_yaxis() 

plt.show()
