# PSI-Microcontroladores2-Aula07
Atividade: Semáforos de Pedestres e Veículos
Nome: Bruno Souza Mora
NUSP: 16906121
Link do Repositório: https://github.com/Poli-USP-Microcontroladores2/sem-foros-de-pedestres-e-ve-culos-1-por-2.git

# Fase de Desenvolvimento
## Requisitos
- Cada LED deve ser controlado por uma thread.
- Threads devem utilizar mutex para evitar conflitos.
- Modos de operação:
 - Normal (dia)
 - Noturno
- Travessia acionada pelo botão

# Arquitetura de Sistema
- Utilização do microcontrolador FRDM KL25Z (Arquitetura ARM) e o Zephyr RTOS para a programação do software;
- Interface para programação e monitoramento da saída serial e de LOGS: Visual Studio Code.

# Projeto de Componentes
## Software
- Threads para o controle de cada LED;
- Uso de um MUTEX para os LEDs para garantir a falta de conflito sobre o acesso dos LEDs.

## Hardware
- 2 Microcontroladores FRDM KL25Z;
- LEDs do FRDM KL25Z;
- 1 jumper de conexão entre um pino de output de um microcontrolador com um pino de botão do outro;
- 1 jumper de conexão entre os terras do microcontrolador;
- 1 jumper para simular o botão.


# Fase de Testes:
## Teste de Unidade - Sistema de pedestres:

https://github.com/user-attachments/assets/87b810f7-6033-407f-99f0-6055869629ed


-<img width="478" height="121" alt="image" src="https://github.com/user-attachments/assets/a1175af9-9b51-44e0-9339-181710554891" />
-  Tempo do LED Vermelho: 4 segundos
- Tempo do LED Verde: 4 segundos
## Teste de Unidade - Sistema de carros:

https://github.com/user-attachments/assets/9c24b115-1722-4554-bb78-bd7b708bf98e
  
-<img width="470" height="119" alt="image" src="https://github.com/user-attachments/assets/80af79e9-92f8-427f-a899-ce579ad3faf2" />

- Tempo do LED Vermelho: 4s
- Tempo do LED Amarelo: 1s
- Tempo do LED Verde: 3s
## Teste de Unidade - Modo Noturno:


https://github.com/user-attachments/assets/ec1a7514-c263-49fe-8bfd-92b5d6ad00e3


- Tempo do intervalo do Toggle no LED amarelo (Carros): 1s (Pisca a cada 2s - 1s aceso e 1s apagado)
- <img width="410" height="53" alt="image" src="https://github.com/user-attachments/assets/a5064dd1-db7b-4445-a4bd-38272d3cb93f" />
![Uploading image.png…]()


https://github.com/user-attachments/assets/b895cf9a-711c-4ae9-b64e-05b891ee9640


- Tempo de intervalo do Toggle do LED vermelho (Pedestres):1s (Pisca a cada 2s - 1s aceso e 1s apagado)
- - <img width="446" height="107" alt="image" src="https://github.com/user-attachments/assets/79b6ebc0-5794-455f-97fc-52d4e333c6c3" />

## Teste de Unidade - Botão de Travessia de Pedestre:

https://github.com/user-attachments/assets/4ca28085-9d26-4ac8-a27c-e31c2af356e5


- Teste se o LED fica vermelho quando o botão é apertado: Ele fica toda vez que o botão é apertado pelo vídeo
- Teste se ele funciona com um contato rápido: Ele funciona em contato rápido
- Teste se ele volta, mesmo estando pressionado: Mesmo sendo continuamente pressionado, ele volta para o ciclo normal.
  
## Teste de Integração e Sistema Integrado - Sincronismo:

https://github.com/user-attachments/assets/d8f93ce1-d95e-42fd-96dc-3e0791037482


- Teste se ambos os faróis estão em sincronismo, com cada um resetando no meio do processo, e testando o funcionamento do botão: Eles funcionam em sincronia, mesmo resetando cada FRDM de maneira diferente. Além disso, elas também estão sincronizadas com o botão.

## Validação
- Apartir do observado, observa-se que os sistema completo atende aos requisitos específicados.

