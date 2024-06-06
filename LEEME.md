# Battle Royale

## Descripción

El mejor evento JcJ para WotLK, creado por [![Lerduzz](https://i.imgur.com/Jhrdgv6.png)](https://youtube.com/@lerduzz).


## Como utilizar

Este sistema se encuentra en desarrollo.


## Requisitos

El Battle Royale requiere:

- SKingCore v0.0.0?


## Instalación

```
1) Colocar en la carpeta MODULES del CORE.
2) Ejecutar CMAKE y compilar nuevamente.
3) Importar ficheros SQL.
```

## Cambiar los ajustes (opcional)

Si necesitas personalizarlo ve a la carpeta de configuracion de los modulos (donde tienes `worldserver` o `worldserver.exe`), copia `battleroyale.conf.dist` to `battleroyale.conf` y edita el nuevo fichero.


## Creditos

* [Yo](https://github.com/Lerduzz) (Creador)


## POR HACER
- Cuando se va a ejecutar la llamada automatica invitar a todos los jugadores que no se encuentren en actividades importantes>
  * En combate.
  * En grupo.
  * En mapa instanciable.
  * En cola de RDF.
  * En cola de BG o Arenas.
  * En conquista del invierno (o en cola para ella).
- En el momento de iniciar la ronda activar el cartel de invitacion como en conquista.
  * Que no se pueda aceptar la invitacion si no se cumplen los requisitos.
- Revisar los mensajes del sistema para que sea AreaTrigger messaje en lugar de notify donde esto funcione.
- Revisar el problema del NPC cuando tratas de unirte a la cola en un mapa especifico que se une random automaticamente.
- Que se muestre en el NPC o en algun sitio el tiempo que falta para el inicio automatico de la ronda.
- Que se muestre en algun sitio la cantidad de jugadores en cola y la cantidad requerida.
- IMPORTANTE: Revisar el problema del CRASH, tal vez si se adapta el sistema para que funcione como conquista ya esto no suceda.
- Hacer que en el mensaje no salga CONQUISTA DEL INVIERNO.
