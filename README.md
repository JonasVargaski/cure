TODO:
failFlagsBlockParam : Adicionar na tela
wifiSignalQuality: intensidade do sinal wifi do dispositivo (0=DESCONHECIDO 1=RUIM, 2=MEDIO, 3=BOM, 4=ÓTIMO) (ajustar na tela mais um parametro)
wifiDeviceId(0x1450, &variableMutex, 14); Alterar tamanho para 14
remotePasswordParam: ajustar na tela range de 0 a 9999
firmwareVersion: adicionar na tela versao do software
Adicionar parametro para habilitar modo de segurança quando falta energia
Ajustar tempo para religar alarme automaticamente, 0 nao funciona, liga na hora

Adicionar tempo de flap aberto e fechado para modo de segurança
Remover parametro de intervalo de acionamentos da ventoinha, deixar fixo em código.
remover filtro de dados remotos para wifi.

implementar indicador de alarme, injectionMachineOutputStateA
deve exibir um alarme a cada 2 segundos quando houver mais de 1

implementar tipo de sensor de umidade (UR , %) e mudar calculo

Implementar indicador de humidade alta/baixa

Corrigir flag injectionMachineOutputStateA, agora é somente ligado e desligado

mac: 3C:71:BF:44:72:94 | 3C71BF447294
[MQTT] params: /cure/3c71bf447294, receiveParams: /cure/3c71bf447294:1002

/cure/08b61f9bc910:0005

https://github.com/JonasVargaski/cure/raw/main/firmware.bin
