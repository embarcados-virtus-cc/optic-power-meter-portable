import matplotlib.pyplot as plt

# Dados extraídos da segunda imagem (1550 nm)
att_voa = [0, 1, 2, 3, 4, 5, 6, 7]
power_meter = [-11.25, -11.61, -12.54, -13.44, -14.38, -15.31, -16.26, -17.22]
sfp_solucao = [None, -11.4, -12.3, -13.2, -14.1, -15.1, -16.0, -40.0] # None para o valor vazio em 0dB
erro_percentual = [None, 1.81, 1.91, 1.80, 1.95, 1.37, 1.60, None]

# Criando a figura com dois subplots (Potência e Erro)
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 10), sharex=True)

# --- Gráfico 1: Potência ---
ax1.plot(att_voa, power_meter, marker='o', label='Power Meter (dBm)', color='#1f77b4')
# Filtrando o None para o plot do SFP não quebrar
sfp_plot_val = [val if val is not None else float('nan') for val in sfp_solucao]
ax1.plot(att_voa, sfp_plot_val, marker='s', label='Solução SFP (dBm)', color='#d62728', linestyle='--')

ax1.set_ylabel('Potência (dBm)')
ax1.set_title('Medições em 1550 nm: Power Meter vs SFP')
ax1.legend()
ax1.grid(True, alpha=0.3)

# --- Gráfico 2: Erro Percentual ---
# Filtrando o None para o plot de erro
erro_plot_val = [val if val is not None else float('nan') for val in erro_percentual]
ax2.bar(att_voa, erro_plot_val, color='#2ca02c', alpha=0.7, label='Erro (%)')

ax2.set_xlabel('Atenuação VOA (dB)')
ax2.set_ylabel('Erro (%)')
ax2.set_ylim(0, 3) # Ajuste de escala para ver melhor os erros pequenos
ax2.legend()
ax2.grid(True, axis='y', alpha=0.3)

plt.tight_layout()
plt.show()
