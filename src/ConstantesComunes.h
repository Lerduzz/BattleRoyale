#ifndef SC_BR_CONST_H
#define SC_BR_CONST_H

#include "Player.h"

enum BR_Dimensiones
{
    DIMENSION_NORMAL                        = 0x00000001,
    DIMENSION_EVENTO                        = 0x00000002,
};

enum BR_EstadosEvento
{
    ESTADO_NO_HAY_SUFICIENTES_JUGADORES     = 0,
    ESTADO_INVOCANDO_JUGADORES              = 1,
    ESTADO_NAVE_EN_MOVIMIENTO               = 2,
    ESTADO_NAVE_CERCA_DEL_CENTRO            = 3,
    ESTADO_BATALLA_EN_CURSO                 = 4,
    ESTADO_BATALLA_TERMINADA                = 5,
};

enum BR_Hechizos
{
    HECHIZO_ALAS_MAGICAS                    = 24742,
    HECHIZO_ANTI_SANADORES                  = 55593,
    HECHIZO_ANTI_INVISIBLES                 = 34709,
};

enum BR_ObjetosMundo
{
    OBJETO_NAVE                             = 194675,
    OBJETO_COFRE                            = 499999,
    OBJETO_CENTRO_DEL_MAPA                  = 500000,
    OBJETO_ZONA_SEGURA_INICIAL              = 500001,
};

const int CANTIDAD_DE_ZONAS                 = 10;
const float BR_EscalasDeZonaSegura[CANTIDAD_DE_ZONAS] = { 5.0f, 4.5f, 4.0f, 3.5f, 3.0f, 2.5f, 2.0f, 1.5f, 1.0f, 0.5f };

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

const int CANTIDAD_DE_MAPAS = 4;
const int BR_IdentificadorDeMapas[CANTIDAD_DE_MAPAS] = { 1, 0, 1, 1 };

const Position BR_CentroDeMapas[CANTIDAD_DE_MAPAS] =
{
    { 5261.581055f, -2164.183105f, 1259.483765f },
    { -14614.625001f, -313.262604f, 0.000001f },
    { 157.995544f, -1948.086061f, 87.387062f },
    { -4695.309082f, 3411.214844f, 7.050505f }
};

const std::string BR_NombreDeMapas[CANTIDAD_DE_MAPAS] =
{
    "Kalimdor: Hyjal",
    "Reinos del Este: Isla Jaguero",
    "Kalimdor: Las Charcas del Olvido",
    "Kalimdor: Ruinas de Solarsal"
};

const float BR_InicioDeLaNave[CANTIDAD_DE_MAPAS][4] =
{
    { 2967.581055f, -2164.183105f, 1556.483765f, 0.0f - M_PI / 2.0f },
    { -12320.625001f, -313.262604f, 297.000001f, 0.0f + M_PI / 2.0f },
    { -2137.581055f, -1948.086061f, 394.387062f, 0.0f - M_PI / 2.0f },
    { -2401.309082f, 3411.214844f, 304.050505f, 0.0f + M_PI / 2.0f }
};

#endif