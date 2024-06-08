#ifndef SC_BR_CONST_H
#define SC_BR_CONST_H

#include "Player.h"

enum BREstado
{
    ESTADO_BR_SIN_SUFICIENTES_JUGADORES = 0,    // Esperando a que se llene la cola o el inicio programado (30min).
    ESTADO_BR_INVITANDO_JUGADORES,              // Invitando jugadores de la cola hasta llenar el evento (30seg).
    ESTADO_BR_ESPERANDO_JUGADORES,              // Esperando que los jugadores terminen de aceptar el llamado (15seg).
    ESTADO_BR_NAVE_EN_ESPERA,                   // Esperando que los jugadores salgan de pantalla de carga y compren armas (30seg).
    ESTADO_BR_NAVE_EN_MOVIMIENTO,               // La nave avanza hacia la zona de la batalla (30seg).
    ESTADO_BR_NAVE_POR_DESAPARECER,             // La nave esta girando para desaparecer (15seg).
    ESTADO_BR_ZONA_EN_ESPERA,                   // BATALLA EN CURSO: Esperando para reducir la zona (60s) x5.
    ESTADO_BR_ZONA_EN_REDUCCION,                // BATALLA EN CURSO: Esperando para reducir la zona (30s) x5.
    ESTADO_BR_ZONA_DESAPARECIDA,                // BATALLA EN CURSO: Todos reciben daño de zona hasta que quede solo 1 con vida (~20seg).
    ESTADO_BR_BATALLA_TERMINADA,                // Batalla terminada, esperando para cerrar el evento (10seg).
}

enum BR_EstadosEvento
{
    ESTADO_NO_HAY_SUFICIENTES_JUGADORES = 0,
    ESTADO_INVOCANDO_JUGADORES,
    ESTADO_NAVE_EN_MOVIMIENTO,
    ESTADO_NAVE_CERCA_DEL_CENTRO,
    ESTADO_BATALLA_EN_CURSO,
    ESTADO_BATALLA_TERMINADA,
};

enum BR_EstadosZona
{
    ESTADO_ZONA_SIN_EVENTO = 0,
    ESTADO_ZONA_EN_ESPERA,
    ESTADO_ZONA_EN_REDUCCION,
    ESTADO_ZONA_DESAPARECIDA,
};

enum BR_Hechizos
{
    HECHIZO_PARACAIDAS                      = 45472,
    HECHIZO_PARACAIDAS_EFECTO               = 44795,
    HECHIZO_ALAS_MAGICAS                    = 24742,
    HECHIZO_ANTI_SANADORES                  = 55593,
    HECHIZO_ANTI_INVISIBLES                 = 34709,
    HECHIZO_RAYO_DRAGON                     = 44318,
    HECHIZO_RAYO_DRAGON_FUERTE              = 46380,
    HECHIZO_BENEFICIO_LIEBRE                = 51442,
    HECHIZO_ACIDO_ZONA                      = 26476,
    HECHIZO_RASTRILLO_LENTO                 = 22639,
    HECHIZO_DESGARRO_ASESINO                = 65033,
};

const int CANTIDAD_DE_VARIACIONES = 49;
const float BR_VariacionesDePosicion[CANTIDAD_DE_VARIACIONES][2] =
{
    { -4.5f,  4.5f }, { -3.0f,  4.5f }, { -1.5f,  4.5f }, {  0.0f,  4.5f }, {  1.5f,  4.5f }, {  3.0f,  4.5f }, {  4.5f,  4.5f },
    { -4.5f,  3.0f }, { -3.0f,  3.0f }, { -1.5f,  3.0f }, {  0.0f,  3.0f }, {  1.5f,  3.0f }, {  3.0f,  3.0f }, {  4.5f,  3.0f },
    { -4.5f,  1.5f }, { -3.0f,  1.5f }, { -1.5f,  1.5f }, {  0.0f,  1.5f }, {  1.5f,  1.5f }, {  3.0f,  1.5f }, {  4.5f,  1.5f },
    { -4.5f,  0.0f }, { -3.0f,  0.0f }, { -1.5f,  0.0f }, {  0.0f,  0.0f }, {  1.5f,  0.0f }, {  3.0f,  0.0f }, {  4.5f,  0.0f },
    { -4.5f, -1.5f }, { -3.0f, -1.5f }, { -1.5f, -1.5f }, {  0.0f, -1.5f }, {  1.5f, -1.5f }, {  3.0f, -1.5f }, {  4.5f, -1.5f },
    { -4.5f, -3.0f }, { -3.0f, -3.0f }, { -1.5f, -3.0f }, {  0.0f, -3.0f }, {  1.5f, -3.0f }, {  3.0f, -3.0f }, {  4.5f, -3.0f },
    { -4.5f, -4.5f }, { -3.0f, -4.5f }, { -1.5f, -4.5f }, {  0.0f, -4.5f }, {  1.5f, -4.5f }, {  3.0f, -4.5f }, {  4.5f, -4.5f }
};

struct BRConf_Recompensa
{
    uint32 base;
    uint32 victoria;
    uint32 asesinar;
    uint32 serAsesinado;
    uint32 morir;
    uint32 zona;
};

#endif
