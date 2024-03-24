Alarme:

[x] 1. VP para indicar se está habilitado ou não
[] 2. VP para indicar se está ligado ou nao (estado da saída)
[] 3. método setValue da habilitar/desabilitar e salvar nas preferencias
[] 4. método getValue para atualizar o display (1. habilitado/desabilitado)
[] 5. método getState para atualizar o display (2. estado da saída)
[] 6. método handle para fazer a lógica do tempo ligado/desligado e escrever no pino;
[] 7. método setEnabled para ligar o alarme

Se alarme desparou, e foi desligado manualmente, quando temperatura atingir setPoint e tenha um tempo definido para religar automaticamente, alarme liga novamente.
Se alarme desligado e tenha um tempo definido para religar, tempo de religar começa a contar... quando atingir liga novamente.
