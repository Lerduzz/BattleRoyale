#ifndef SC_BR_CHAT_MGR_H
#define SC_BR_CHAT_MGR_H

#include "Constantes.h"
#include "Chat.h"

class MensajeMgr
{
    MensajeMgr(){};
    ~MensajeMgr(){};

public:
    static MensajeMgr *instance()
    {
        static MensajeMgr *instance = new MensajeMgr();
        return instance;
    }

    void NotificarTiempoInicial(uint32 tiempo, BRListaPersonajes lista, std::string mapa = "Lugar: Desconocido");
    void NotificarAdvertenciaDeZona(uint32 tiempo, BRListaPersonajes lista);
    void NotificarZonaEnReduccion(BRListaPersonajes lista);
    void NotificarNaveRetirada(BRListaPersonajes lista);
    void AnunciarJugadoresEnCola(Player *player, uint32 minimo, BRListaPersonajes lista, BRMensaje estado = BR_MENSAJE_ESTADO_EVENTO_OK);
    void AnunciarMuerteJcJ(Player *killer, Player *killed, int kills, BRListaPersonajes lista);
    void AnunciarGanador(Player *winner, int kills);
    void AnunciarEmpate();
    void AnunciarEfectoZona(BRListaPersonajes lista, int vivos);
    void AnunciarConteoCofres(int cofres, BRListaPersonajes lista);
    void AnunciarMensajeBienvenida(Player *player);
    void AnunciarErrorAlas(Player *player);
    void AnunciarMensajeEntrada(Player *player, BRMensaje tipo);
    void AnunciarAvisoInicioForzado();
    void AnunciarErrorInicioForzado();
    void AnunciarInicioForzado(uint32 count);

private:
    inline ChatHandler Chat(Player *player) { return ChatHandler(player->GetSession()); };
    void AnunciarRondaIniciada(std::string mapa, uint32 cantidad);
};

#define sMensajeMgr MensajeMgr::instance()

#endif
