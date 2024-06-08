#ifndef SC_BR_CONST_H
#define SC_BR_CONST_H

#include "Player.h"

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
