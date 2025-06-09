# Passo a passo: Captura de telas dos traces com KernelShark

Este guia detalha como capturar as imagens necessárias para análise dos traces de escalonamento usando o KernelShark.

## 1. Pré-requisitos
- KernelShark instalado (instale com `sudo apt install kernelshark` se necessário)
- Os arquivos de trace (`trace_fifo_v3.dat`, `trace_rr_v3.dat`, `trace_other_v3.dat`, `trace_idle_v3.dat`, `trace_low_idle_v3.dat`) presentes na pasta do projeto

## 2. Para cada arquivo de trace:
Repita os passos abaixo para **cada** arquivo `.dat`:
- `trace_fifo_v3.dat`  (SCHED_FIFO)
- `trace_rr_v3.dat`    (SCHED_RR)
- `trace_other_v3.dat` (SCHED_OTHER)
- `trace_idle_v3.dat`  (SCHED_IDLE)
- `trace_low_idle_v3.dat` (SCHED_LOW_IDLE)

### Passo a passo detalhado

1. **Abra o KernelShark**
   - No terminal, execute:
     ```bash
     kernelshark
     ```

2. **Carregue o arquivo de trace**
   - No menu do KernelShark, clique em `File > Open`.
   - Selecione o arquivo `.dat` correspondente à política desejada.

3. **Filtre apenas o evento `sched:sched_switch`**
   - No painel esquerdo, em `Events`, desmarque todos e marque apenas `sched:sched_switch`.

4. **Filtre apenas as threads do seu programa**
   - No painel esquerdo, em `Tasks`, marque apenas os PIDs das threads do seu programa (A, B, C, D).
   - Dica: Se não souber os PIDs, veja na List View após abrir o trace. Os nomes das threads geralmente aparecem junto ao PID.

5. **Filtre apenas a CPU 0**
   - No painel esquerdo, em `CPUs`, marque apenas a CPU 0.

6. **Capture as telas necessárias**
   - **Graph View (Timeline):**
     - Ajuste o zoom para mostrar a linha do tempo das threads.
     - Capture a tela (PrintScreen ou ferramenta de captura).
     - Salve como, por exemplo: `kernelshark_sched_fifo.png`
   - **List View (Eventos):**
     - Abra a List View (ícone de lista ou menu `View > List`).
     - Capture a tela mostrando os eventos `sched:sched_switch` das threads.
   - **Zoom em períodos específicos:**
     - Na Graph View, selecione um trecho com várias trocas de contexto.
     - Dê zoom e capture a tela.

7. **Repita para todos os arquivos de trace**
   - Faça o mesmo procedimento para cada arquivo `.dat`.

8. **Comparação entre políticas**
   - Junte as imagens das timelines de cada política em um editor de imagens (GIMP, Paint, etc).
   - Salve como `kernelshark_comparison.png`.

## 3. Checklist de nomes dos arquivos
- `kernelshark_sched_fifo.png`
- `kernelshark_sched_rr.png`
- `kernelshark_sched_other.png`
- `kernelshark_sched_idle.png`
- `kernelshark_sched_low_idle.png`
- `kernelshark_comparison.png`

## 4. Dicas rápidas
- Use o zoom e pan para destacar execuções e trocas de contexto.
- Certifique-se de que os nomes das threads/PIDs estejam visíveis nas capturas.
- Se quiser, anote os PIDs das threads para facilitar os filtros nos próximos arquivos.

---

Se precisar de ajuda para identificar os PIDs das threads em cada trace, consulte a List View do KernelShark logo após abrir o arquivo. 