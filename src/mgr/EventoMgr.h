#ifndef SC_BR_EVENTO_MGR_H
#define SC_BR_EVENTO_MGR_H

#include "Constantes.h"
#include "CharacterCache.h"
#include "ChatMgr.h"
#include "MapaMgr.h"

class EventoMgr
{
    EventoMgr();
    ~EventoMgr();

public:
    static EventoMgr *instance()
    {
        static EventoMgr *instance = new EventoMgr();
        return instance;
    }

    /**
     * @brief Determina si un jugador cumple las condiciones para entrar en la cola del evento.
     *
     * @param player El jugador que esta intentando unirse a la cola.
     * @return true
     * @return false
     */
    bool PuedeEntrarCola(Player *player);

    /**
     * @brief Determina si el jugador se encuentra en la cola para el evento.
     *
     * @param player El jugador que se quiere comprobar.
     * @return true
     * @return false
     */
    bool EstaEnCola(Player *player);

    /**
     * @brief Determina si el jugador esta invitado al evento.
     *
     * @param player El jugador que se quiere comprobar.
     * @return true
     * @return false
     */
    bool EstaInvitado(Player *player);

    /**
     * @brief Determina si el jugador se encuentra dentro del evento.
     *
     * @param player El jugador que se quiere comprobar.
     * @return true
     * @return false
     */
    bool EstaEnEvento(Player *player);

    /**
     * @brief Gestiona al jugador que quiere entrar al evento.
     *
     * @param player EL jugador que quiere entrar al evento.
     */
    void JugadorEntrando(Player *player);

    /**
     * @brief Gestiona cuando un jugador debe salir del evento.
     * 
     * @param player Jugador que esta saliendo.
     * @param logout Bandera para saber si la salida es por desconexion.
     */
    void JugadorSaliendo(Player *player, bool logout = false);

private:
    /**
     * @brief Establece todas las variables del evento a su estado inicial.
     *
     */
    void LimpiarEvento();

    /**
     * @brief Prepara una nueva ronda y comienza a invitar jugadores.
     *
     */
    void IniciarRonda();

    /**
     * @brief Invita a los jugadores al evento mientras se cumplan los requisitos.
     *
     */
    void InvitarJugadores();

    /**
     * @brief Invita al jugador a la batalla, ya debe estar en la lista.
     *
     * @param player El jugador a invitar.
     */
    void InvitarJugador(Player *player);

    /**
     * @brief Almacena el estado actual en el que se encuentra el evento.
     *
     */
    BREstado estado;

    /**
     * @brief Almacena la cola para el evento.
     *
     */
    BRCola cola;

    /**
     * @brief Almacena la lista de personajes que han sido invitados al evento.
     *
     */
    BRListaPersonajes invitados;

    /**
     * @brief Almacena la lista de personajes dentro del evento.
     *
     */
    BRListaPersonajes jugadores;

    /**
     * @brief Tiempo restante para que inicie el combate desde el momento en que se comienza a invitar jugadores.
     *
     */
    uint32 tiempoRestanteInicio;

    /**
     * @brief Determina la variacion de posicion del jugador invitado para que se invoquen en formacion.
     *
     */
    uint32 indiceDePosicionNave;

    // ---------------------------- Variables que se deben cargar desde configuracion --------------------------------
    BRConfigIntervalos intervalo;
    uint32 minJugadores;
    uint32 maxJugadores;
};

/**
 * @brief Se encarga de gestionar globalmente todo el evento.
 *
 */
#define sEventoMgr EventoMgr::instance()

#endif
