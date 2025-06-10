## -=-=-=--=-=-=--=-=-=--=-=-=--=-=-=--=-=-=--=-=-=--=-=-=--=-=-=--=
# RELATÓRIO DE ANÁLISE DE POLÍTICAS DE ESCALONAMENTO COM KERNELSHARK
**Alunos**: Alice Colares e Bernardo Heitz  
**Disciplina**: Laboratório de Sistemas Operacionais  
**Data**: Junho 2025 

## -=-=-=--=-=-=- Introdução -=-=-=--=-=-=-

Este relatório apresenta uma análise detalhada do comportamento de diferentes políticas de escalonamento de threads no Linux, utilizando traces coletados e visualizados com o KernelShark. O objetivo é compreender como cada política afeta a alternância entre threads, a duração dos períodos de execução e o padrão de preempção.

---

## -=-=-=--=-=-=- Descrição do programa -=-=-=--=-=-=-

O **sched_profiler** foi desenvolvido para analisar e demonstrar o comportamento de diferentes políticas de escalonamento do kernel Linux. O programa cria múltiplas threads que escrevem em um buffer compartilhado, permitindo visualizar como o escalonador do sistema operacional alterna entre as threads sob diferentes tipos de políticas de escalonamento.

- Cada thread possui um caractere identificador único (A, B, C, D...)
- Todas as threads são escritas no mesmo buffer global de forma sincronizada
- O padrão resultante no buffer reflete a sequência de execução das threads
- O programa realiza pós-processamento para agregar períodos consecutivos
- Estatísticas de execução são coletadas e apresentadas pelo nosso programa

### Parâmetros de entrada:

```
./sched_profiler <tamanho_buffer> <num_threads> <policy>
```

- **tamanho_buffer**: Tamanho do buffer: (inteiro > 0)
- **num_threads**: Número de threads que serão criadas: (1-26)
- **policy**: Política de escalonamento:
  - `0` = SCHED_OTHER (padrão do sistema)
  - `1` = SCHED_FIFO (tempo real, first-in-first-out)
  - `2` = SCHED_RR (tempo real, round-robin)
  - `5` = SCHED_IDLE (baixa prioridade)
  - `7` = SCHED_LOW_IDLE (prioridade customizada, mais baixa que IDLE)

#### Exemplos de uso:

```
# Teste com política padrão
./sched_profiler 500 4 0

# Teste com política tempo real FIFO
./sched_profiler 500 4 1

# Teste com política IDLE
./sched_profiler 500 4 5
```

---

## Realização do trabalho:

Para esta análise, foi utilizado o programa `sched_profiler`, que cria múltiplas threads (A, B, C, D, E) e executa sob diferentes políticas de escalonamento: **SCHED_FIFO**, **SCHED_RR**, **SCHED_OTHER**, **SCHED_IDLE** e **SCHED_LOW_IDLE**.

**Ambiente de execução:**
- Os testes foram realizados em um ambiente single-core "CPU(0)".
- No kernelShark, os filtros de CPU foram ajustados conforme a visualização desejada, mantendo apenas a opção de single-core, eles erão os únicos selecionáveis no kernelshark.
- Isso perite observar o comportamento de escalonamento em um único core.

**Procedimento:**
- **Coleta dos traces:** Para cada política, o programa foi executado e os eventos do kernel foram registrados.
- **Visualização:** Os arquivos de trace foram abertos no kernelShark, aplicando os filtros:
  - Evento: Apenas `sched:sched_switch`
  - Tasks: Apenas os PIDs das threads do `sched_profiler`
  - CPU: Seleção de CPUs conforme análise (em geral, deixando apenas a opção CPU(0), pois ela erá a único disponível, visto que estamos realizando o trabalho em um sistema single-core)
- **Captura de imagens:** Para cada política, foram capturadas:
  - **Graph view:** Linha do tempo das threads.
  - **List view:** Lista de eventos `sched:sched_switch`.
  - **Zoom:** Segmento com muitas trocas de contexto.

---

## -=-=-=--=-=-=- Exemplos de saída -=-=-=--=-=-=-

### Entrada:
```
./sched_profiler 500 4 1
```

### Saída:
```
=== SCHED_PROFILER ===
Política: SCHED_FIFO (1)
Threads: 4
Buffer size: 500
Iniciando execução...

Execução concluída!

Saída sem pós-processamento:
AAAAAAAAAABBBBBBBBBBCCCCCCCCCCDDDDDDDDDDAAAAAAAAAA
BBBBBBBBBBCCCCCCCCCCDDDDDDDDDDAAAAAAAAAABBBBBBBBBB
...

Saída após pós-processamento:
ABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDAB

Contagem de períodos de execução:
A = 13
B = 13
C = 12
D = 12

```

---

## -=-=-=--=-=-=- Resultados por política -=-=-=--=-=-=-

### 3.1 SCHED_FIFO: 

- **Graph view:** ![Graph view FIFO](kernelshark_sched_fifo.png)
- **List view:** ![List view FIFO](kernelshark_sched_fifo_list.png), ![List view FIFO 2](kernelshark_sched_fifo_list2.png)
- **Zoom em trocas rápidas:** ![Zoom FIFO](kernelshark_sched_fifo_zoom.png)

**Análise:**  
O escalonador SCHED_FIFO (First-In, First-Out) é uma política de tempo real baseada em prioridades fixas. Observa-se na timeline que as threads são executadas em blocos longos e previsíveis, com poucas interrupções. As trocas de contexto ocorrem apenas quando uma thread de maior prioridade se torna pronta ou quando uma thread termina sua execução. Isso resulta em padrões altamente regulares, com cada thread ocupando a CPU por períodos uniformes. Não há preempção inesperada, o que garante previsibilidade, mas pode levar a starvation de threads de menor prioridade.

---

### 3.2 SCHED_RR:

- **Graph view:** ![Graph view RR](kernelshark_sched_rr.png)
- **List view:** ![List view RR](kernelshark_sched_rr_list.png), ![List view RR 2](kernelshark_sched_rr_list2.png)
- **Zoom em trocas rápidas:** ![Zoom RR](kernelshark_sched_rr_zoom.png)

**Análise:**  
A política SCHED_RR (Round Robin) também é de tempo real, mas introduz fatias de tempo para cada thread de mesma prioridade. Na timeline, é possível ver alternância frequente e ordenada entre as threads, com cada uma recebendo uma fatia de CPU antes de ser preemptada pela próxima. Os períodos de execução são uniformes e as trocas de contexto são regulares, promovendo justiça entre as threads. O comportamento é previsível, ideal para aplicações que exigem resposta rápida e balanceamento entre tarefas de igual prioridade.

---

### 3.3 SCHED_OTHER:

- **Graph view:** ![Graph view OTHER](kernelshark_sched_other.png)
- **List view:** ![List view OTHER](kernelshark_sched_other_list.png), ![List view OTHER 2](kernelshark_sched_other_list2.png)
- **Zoom em trocas rápidas:** ![Zoom OTHER](kernelshark_sched_other_zoom.png)

**Análise:**  
O SCHED_OTHER é a política padrão do Linux, baseada em tempo compartilhado (CFS - Completely Fair Scheduler). A timeline revela padrões irregulares, com variação significativa nos períodos de execução das threads. A preempção é dinâmica, ocorrendo conforme o uso recente de CPU de cada thread, o que pode resultar em alternância menos previsível e intervalos de execução variáveis. Isso reflete o objetivo do CFS de distribuir o tempo de CPU de forma justa, mas pode causar latências imprevisíveis em aplicações sensíveis a tempo real.resultante

---

### 3.4 SCHED_IDLE:

- **Graph view:** ![Graph view IDLE](kernelshark_sched_idle.png)
- **List view:** ![List view IDLE](kernelshark_sched_idle_list.png), ![List view IDLE 2](kernelshark_sched_idle_list2.png)
- **Zoom em trocas rápidas:** ![Zoom IDLE](kernelshark_sched_idle_zoom.png)

**Análise:**  
A política SCHED_IDLE funcionade de modo que, as threads só devem executar quando o sistema está ocioso. Na timeline, observa-se longos períodos de inatividade das threads, que só são executadas quando não há outras tarefas de maior prioridade. Os períodos de execução são longos, mas esparsos, e as trocas de contexto são raras. Isso é ideal para tarefas de baixa prioridade, como manutenção ou processamento em background, que não devem interferir no desempenho do sistema.

---

### 3.5 SCHED_LOW_IDLE:

- **Graph view:** ![Graph view LOW_IDLE](kernelshark_sched_low_idle.png)
- **List view:** ![List view LOW_IDLE](kernelshark_sched_low_idle_list.png), ![List view LOW_IDLE 2](kernelshark_sched_low_idle_list2.png)
- **Zoom em trocas rápidas:** ![Zoom LOW_IDLE](kernelshark_sched_low_idle_zoom.png)

**Análise:**  
O SCHED_LOW_IDLE (quando disponível) funciona de forma semelhante ao SCHED_IDLE, priorizando ainda menos as threads. A timeline mostra execuções ainda mais espaçadas e trocas de contexto muito raras. As threads só rodam quando absolutamente nenhuma outra tarefa está pronta, tornando essa política adequada para processos que podem esperar indefinidamente sem impacto no sistema.

---

## -=-=-=--=-=-=- Comparação entre políticas -=-=-=--=-=-=-

- **Imagem comparativa:** ![Comparação entre políticas](kernelshark_comparison.png)

**Análise da comparação:**  
A imagem acima apresenta, lado a lado, as timelines das diferentes políticas de escalonamento. É possível observar claramente as diferenças de comportamento:

- **FIFO e RR:** As execuções são regulares, com alternância previsível entre as threads. No FIFO, cada thread executa por longos períodos, enquanto no RR há alternância mais frequente devido à fatia de tempo.
- **OTHER:** A alternância entre threads é mais dinâmica e menos previsível, com variações nos períodos de execução e preempções frequentes, refletindo o funcionamento do CFS.
- **IDLE e LOW_IDLE:** As execuções são mais esparsas e acabam ocorrendo apenas quando o sistema está ocioso. Os períodos de inatividade são longos, e as trocas de contexto são raras, evidenciando a baixa prioridade dessas políticas.

---

## Execução do programa sched_profiler:

O programa `sched_profiler` foi utilizado para gerar os traces analisados neste relatório. O código-fonte está disponível no arquivo `sched_profiler.c`.

### -=-=-=--=-=-=- Exemplo de execução e saída para cada política -=-=-=--=-=-=-

#### SCHED_OTHER:
```
# trace-cmd record -o trace_other_v3.dat -e sched:sched_switch ./sched_profiler 500 4 0
=== SCHED_PROFILER ===
Política: SCHED_OTHER (0)
Threads: 4
Buffer size: 500
Iniciando execução...

Execução concluída!

Saída sem pós-processamento:
DDDDDDDDDDCCCCCCCCCCBBBBBBBBBBAAAAAAAAAADDDDDDDDDD...

Saída após pós-processamento:
DCBADCBACDBACDBACDBACDBACDBACDBACDBADBCADBCADBCADB

Contagem de períodos de execução:
A = 12
B = 13
C = 12
D = 13

Estatísticas de escalonamento (total de escritas):
A = 120
B = 130
C = 120
D = 130

Distribuição percentual:
A = 24.0%
B = 26.0%
C = 24.0%
D = 26.0%
```

#### SCHED_FIFO:
```
# trace-cmd record -o trace_fifo_v3.dat -e sched:sched_switch ./sched_profiler 500 4 1
=== SCHED_PROFILER ===
Política: SCHED_FIFO (1)
Threads: 4
Buffer size: 500
Iniciando execução...

Execução concluída!

Saída sem pós-processamento:
AAAAAAAAAABBBBBBBBBBCCCCCCCCCCDDDDDDDDDDAAAAAAAAAA...

Saída após pós-processamento:
ABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDAB

Contagem de períodos de execução:
A = 13
B = 13
C = 12
D = 12

Estatísticas de escalonamento (total de escritas):
A = 130
B = 130
C = 120
D = 120

Distribuição percentual:
A = 26.0%
B = 26.0%
C = 24.0%
D = 24.0%
```

#### SCHED_RR:
```
# trace-cmd record -o trace_rr_v3.dat -e sched:sched_switch ./sched_profiler 500 4 2
=== SCHED_PROFILER ===
Política: SCHED_RR (2)
Threads: 4
Buffer size: 500
Iniciando execução...

Execução concluída!

Saída sem pós-processamento:
AAAAAAAAAABBBBBBBBBBCCCCCCCCCCDDDDDDDDDDAAAAAAAAAA...

Saída após pós-processamento:
ABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDAB

Contagem de períodos de execução:
A = 13
B = 13
C = 12
D = 12

Estatísticas de escalonamento (total de escritas):
A = 130
B = 130
C = 120
D = 120

Distribuição percentual:
A = 26.0%
B = 26.0%
C = 24.0%
D = 24.0%
```

#### SCHED_IDLE: 
```
# trace-cmd record -o trace_idle_v3.dat -e sched:sched_switch ./sched_profiler 500 4 5
=== SCHED_PROFILER ===
Política: SCHED_IDLE (5)
Threads: 4
Buffer size: 500
Iniciando execução...

Aviso: Erro ao definir política (pode necessitar privilégios root): Success
...
Execução concluída!

Saída sem pós-processamento:
CCCCCCCCCCBBBBBBBBBBDDDDDDDDDDAAAAAAAAAABBBBBBBBBB...

Saída após pós-processamento:
CBDABCABDCADBCADCBADCBADCBADCBADCBADCBADCBADCBADCB

Contagem de períodos de execução:
A = 12
B = 13
C = 13
D = 12

Estatísticas de escalonamento (total de escritas):
A = 120
B = 130
C = 130
D = 120

Distribuição percentual:
A = 24.0%
B = 26.0%
C = 26.0%
D = 24.0%
```

#### SCHED_LOW_IDLE:
```
# trace-cmd record -o trace_low_idle_v3.dat -e sched:sched_switch ./sched_profiler 500 4 7
=== SCHED_PROFILER ===
Política: SCHED_LOW_IDLE (7)
Threads: 4
Buffer size: 500
Iniciando execução...

Aviso: Erro ao definir política (pode necessitar privilégios root): Success
...
Execução concluída!

Saída sem pós-processamento:
CCCCCCCCCCDDDDDDDDDDBBBBBBBBBBDDDDDDDDDDCCCCCCCCCC...

Saída após pós-processamento:
CDBDCABDCABDACBACDBACDBADCBADCBADCBADCBADCBADCBDAC

Contagem de períodos de execução:
A = 12
B = 12
C = 13
D = 13

Estatísticas de escalonamento (total de escritas):
A = 120
B = 120
C = 130
D = 130

Distribuição percentual:
A = 24.0%
B = 24.0%
C = 26.0%
D = 26.0%
```

### Referência ao código-conte:

O código-fonte do código utilizado para esse trabalho, está no arquivo `sched_profiler.c`. Ele implementa a criação de múltiplas threads, sincronização via semáforo, simulação de carga de trabalho e configuração da política de escalonamento para cada thread. O programa também gera estatísticas sobre os traces.

---

## -=-=-=--=-=-=- Implementação técnica -=-=-=--=-=-=-

### Sincronização:
- **Semáforo binário**: Controla acesso ao buffer compartilhado
- **Seção crítica**: Escrita no buffer e incremento do ponteiro
- **Thread safety**: Operações atômicas garantidas

### Estruturas de dados:
```c
char *global_buffer;           // Buffer compartilhado
int buffer_index;              // Índice atual do buffer
int *thread_schedule_count;    // Contador por thread
sem_t mutex;                   // Semáforo de sincronização
```
---

## -=-=-=--=-=-=- Captura de traces -=-=-=--=-=-=-
### Comando trace-cmd:
```
# Montar debugfs
mount -t debugfs none /sys/kernel/debug

# Capturar eventos de escalonamento
trace-cmd record -o trace_<policy>.dat -e sched:sched_switch ./sched_profiler 500 4 <policy>

# Visualizar trace
trace-cmd report trace_<policy>.dat
```

### Arquivos gerados:
- `trace_other_v3.dat` - SCHED_OTHER
- `trace_fifo_v3.dat` - SCHED_FIFO  
- `trace_rr_v3.dat` - SCHED_RR
- `trace_idle_v3.dat` - SCHED_IDLE
- `trace_low_idle_v3.dat` - SCHED_LOW_IDLE

---

**Alunos**: Alice Colares e Bernardo Heitz  
**Disciplina**: Laboratório de Sistemas Operacionais  
## -=-=-=--=-=-=--=-=-=--=-=-=--=-=-=--=-=-=--=-=-=--=-=-=--=-=-=--=
