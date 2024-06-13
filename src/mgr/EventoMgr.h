#ifndef SC_BR_ENTIDAD_MGR_H
#define SC_BR_ENTIDAD_MGR_H

#include "Constantes.h"

class EventoMgr
{
    /**
     * @brief Construye un nuevo objeto EventoMgr.
     *
     */
    EventoMgr();

    /**
     * @brief Destruye el objeto EventoMgr.
     *
     */
    ~EventoMgr();

public:
    /**
     * @brief Obtiene la referencia a la instancia de la clase.
     *
     * @return EventoMgr*
     */
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

private:
    /**
     * @brief Establece todas las variables del evento a su estado inicial.
     *
     */
    void LimpiarEvento();

    /**
     * @brief Almacena el estado actual en el que se encuentra el evento.
     *
     */
    BREstado estado;
};

/**
 * @brief Define acceso directo a la instancia global de la clase.
 *
 */
#define sEventoMgr EventoMgr::instance()

#endif
