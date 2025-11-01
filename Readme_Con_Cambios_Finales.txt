Se usó para el sistema operativo windows.
Comando de ejemplo: .\MV.exe ejemplo.vmx -d   
(-d opcional)

Corección de errores con la escritura en memoria por falta de corrimiento para los numeros negativos, agregué en EscriboEnMemoria: 
offset<<=16;
offset>>=16;

Y error escribiendo los bytes dependiendo del modificador, agregué esta linea
Valor = Valor << (modif*8);
