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
    HECHIZO_PARACAIDAS                      = 45472,
    HECHIZO_PARACAIDAS_EFECTO               = 44795,
    HECHIZO_ALAS_MAGICAS                    = 24742,
    HECHIZO_LENGUAJE_BINARIO                = 50246,
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

typedef std::map<uint32, Position> BR_UbicacionZona;
typedef std::map<uint32, BR_UbicacionZona> BR_Ubicacion;
struct BR_Mapa
{
    uint32 idMapa;
    std::string nombreMapa;
    Position centroMapa;
    Position inicioNave;
    BR_Ubicacion ubicacionesMapa;
};
typedef std::map<uint32, BR_Mapa*> BR_ContenedorMapas;

#endif
