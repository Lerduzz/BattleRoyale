#ifndef SC_BR_CONSTANTES_H
#define SC_BR_CONSTANTES_H

#include "Player.h"

// ----------------------------------- ENUMS ----------------------------------- //
enum BRVisibilidad
{
    BR_VISIBILIDAD_NORMAL = 0x00000001,
    BR_VISIBILIDAD_EVENTO = 0x00000002,
};

enum BRObjeto
{
    BR_OBJETO_NAVE = 194675,
    BR_OBJETO_COFRE = 499999,
};

enum BRCriatura
{
    BR_CRIATURA_GESTOR_COLA = 200000,
    BR_CRIATURA_VENDEDOR_ARMAS = 200001,
    BR_CRIATURA_DRAGON_GUARDIAN = 199999,
};

enum BREstado
{
    BR_ESTADO_SIN_SUFICIENTES_JUGADORES = 0,
    BR_ESTADO_INVITANDO_JUGADORES,
    BR_ESTADO_ESPERANDO_JUGADORES,
    BR_ESTADO_NAVE_EN_ESPERA,
    BR_ESTADO_NAVE_EN_MOVIMIENTO,
    BR_ESTADO_NAVE_POR_DESAPARECER,
    BR_ESTADO_ZONA_EN_ESPERA,
    BR_ESTADO_ZONA_EN_REDUCCION,
    BR_ESTADO_ZONA_DESAPARECIDA,
    BR_ESTADO_BATALLA_TERMINADA,
};

enum BRHechizo
{
    BR_HECHIZO_PARACAIDAS = 45472,
    BR_HECHIZO_PARACAIDAS_EFECTO = 44795,
    BR_HECHIZO_ALAS_MAGICAS = 24742,
    BR_HECHIZO_ANTI_SANADORES = 55593,
    BR_HECHIZO_ANTI_INVISIBLES = 34709,
    BR_HECHIZO_RAYO_DRAGON = 44318,
    BR_HECHIZO_RAYO_DRAGON_FUERTE = 46380,
    BR_HECHIZO_BENEFICIO_LIEBRE = 51442,
    BR_HECHIZO_ACIDO_ZONA = 26476,
    BR_HECHIZO_RASTRILLO_LENTO = 22639,
    BR_HECHIZO_DESGARRO_ASESINO = 65033,
};

enum BRSonido
{
    BR_SONIDO_NAVE_EN_MOVIMIENTO = 8232,
    BR_SONIDO_NAVE_RETIRADA = 8213,
    BR_SONIDO_RONDA_INICIADA = 8173,
    BR_SONIDO_ZONA_TIEMPO = 8192,
    BR_SONIDO_ZONA_REDUCIDA = 8212,
    BR_SONIDO_ALGUIEN_MUERE = 8174,
    BR_SONIDO_GANADOR_HORDA = 8454,
    BR_SONIDO_GANADOR_ALIANZA = 8455,
};

enum BRMensaje
{
    BR_MENSAJE_ERROR_NIVEL = 0,
    BR_MENSAJE_ERROR_DK_INICIO,
    BR_MENSAJE_ERROR_EN_COLA,
    BR_MENSAJE_ERROR_INVITADO,
    BR_MENSAJE_ERROR_EN_EVENTO,
};

enum BREquipoInicial
{
    BR_EQUIPO_INICIAL_CAMISA = 17,
    BR_EQUIPO_INICIAL_PANTALONES = 20902,
};

enum BRMision
{
    MISION_DIARIA_1 = 100000,
};

enum BRMisionReq
{
    MISION_DIARIA_1_REQ_1 = 200003,
};

// ----------------------------------- CONSTS ----------------------------------- //
const std::string BR_NOMBRE_CHAT = "|cffff6633[|cffff0000B|cffff3300attle|cffff0000R|cffff3300oyale|cffff6633]|r";

const int BR_CANTIDAD_VARIACIONES = 49;
const float BR_VARIACIONES_POSICION[BR_CANTIDAD_VARIACIONES][2] = {{-4.5f, 4.5f}, {-3.0f, 4.5f}, {-1.5f, 4.5f}, {0.0f, 4.5f}, {1.5f, 4.5f}, {3.0f, 4.5f}, {4.5f, 4.5f}, {-4.5f, 3.0f}, {-3.0f, 3.0f}, {-1.5f, 3.0f}, {0.0f, 3.0f}, {1.5f, 3.0f}, {3.0f, 3.0f}, {4.5f, 3.0f}, {-4.5f, 1.5f}, {-3.0f, 1.5f}, {-1.5f, 1.5f}, {0.0f, 1.5f}, {1.5f, 1.5f}, {3.0f, 1.5f}, {4.5f, 1.5f}, {-4.5f, 0.0f}, {-3.0f, 0.0f}, {-1.5f, 0.0f}, {0.0f, 0.0f}, {1.5f, 0.0f}, {3.0f, 0.0f}, {4.5f, 0.0f}, {-4.5f, -1.5f}, {-3.0f, -1.5f}, {-1.5f, -1.5f}, {0.0f, -1.5f}, {1.5f, -1.5f}, {3.0f, -1.5f}, {4.5f, -1.5f}, {-4.5f, -3.0f}, {-3.0f, -3.0f}, {-1.5f, -3.0f}, {0.0f, -3.0f}, {1.5f, -3.0f}, {3.0f, -3.0f}, {4.5f, -3.0f}, {-4.5f, -4.5f}, {-3.0f, -4.5f}, {-1.5f, -4.5f}, {0.0f, -4.5f}, {1.5f, -4.5f}, {3.0f, -4.5f}, {4.5f, -4.5f}};

const uint32 BR_EQUIPO_MAXIMO = 209;
const uint32 BR_EQUIPO_LISTA[BR_EQUIPO_MAXIMO] = {41896, 41901, 41907, 42055, 42022, 42056, 42023, 42057, 42024, 42110, 41880, 41884, 41892, 41330, 41331, 41332, 41628, 41633, 41638, 42058, 42026, 42059, 42025, 41233, 41228, 41223, 42027, 42060, 42020, 42061, 42021, 42112, 40966, 40973, 40972, 40877, 40878, 40887, 41877, 41879, 41878, 41827, 41828, 41830, 41050, 41049, 41047, 41068, 41073, 41063, 42316, 28298, 42321, 28299, 42331, 24550, 42489, 34529, 42524, 42345, 28297, 32053, 44417, 24557, 44418, 33716, 42359, 33766, 33763, 28346, 42536, 42530, 42501, 42494, 42446, 40781, 40803, 40820, 40841, 42619, 40860, 41308, 41284, 41319, 41296, 42577, 41273, 41659, 41771, 41676, 41665, 42587, 41713, 42389, 34540, 41314, 41291, 41325, 41302, 42582, 41279, 42226, 28309, 42247, 28310, 42269, 28314, 42279, 28302, 42289, 28307, 42484, 34530, 20335, 42563, 42351, 32450, 42569, 42383, 32055, 41085, 41141, 41155, 41203, 41215, 41950, 41969, 41944, 41957, 41963, 40904, 40925, 40931, 40937, 42613, 40961, 40782, 40802, 40821, 40842, 42851, 40861, 42326, 28300, 41857, 41872, 41852, 41862, 41867, 41919, 41938, 41913, 41925, 41931, 41648, 41765, 41670, 41653, 41681, 40988, 40999, 41011, 41025, 42596, 41036, 41079, 41135, 41149, 41162, 42606, 41209, 40989, 41005, 41017, 41031, 42601, 41042, 42207, 42236, 42241, 28312, 42274, 28305, 42284, 28295, 42259, 28313, 42231, 31985, 42254, 33801, 32003, 42264, 42518, 42512, 42558, 42449, 42001, 42015, 41991, 42003, 42009, 40783, 40801, 40819, 40840, 40859, 18854, 18834};

// ----------------------------------- STRUCTS ----------------------------------- //
struct BRConfigRecompensa
{
    uint32 base;
    uint32 victoria;
    uint32 asesinar;
    uint32 serAsesinado;
    uint32 morir;
    uint32 zona;
};

struct BRConfigIntervalos
{
    uint32 sinJugadores;
    uint32 invitandoJugadores;
    uint32 esperandoJugadores;
    uint32 naveEnEspera;
    uint32 zonaEnEspera;
};

struct BRMapa
{
    uint32 idMapa;
    uint32 idZona;
    std::string nombreMapa;
    Position centroMapa;
    Position inicioNave;
    std::map<uint32, std::map<uint32, Position>> ubicacionesMapa;
    uint32 votos;
};
typedef std::map<uint32, BRMapa *> BRListaMapas;

// ----------------------------------- TYPEDEFS ----------------------------------- //
typedef std::map<uint32, Player *> BRListaPersonajes;
// typedef std::map<uint32, BattleRoyaleData> BRListaDatos;
typedef std::map<uint32, uint32> BRListaVotos;
typedef std::list<Player *> BRCola;

#endif
