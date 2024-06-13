#include "ChatMgr.h"

void ChatMgr::MensajeEntrada(Player *player, BRMensaje tipo, std::string extra)
{
    switch (tipo)
    {
    case BR_MENSAJE_ERROR_NIVEL:
    {
        Chat(player).PSendSysMessage("%s ¡Este modo de juego es solo para personajes de nivel 80!", BR_NOMBRE_CHAT.c_str());
        break;
    }
    case BR_MENSAJE_ERROR_DK_INICIO:
    {
        Chat(player).PSendSysMessage("%s ¡Primero debes terminar tu cadena de misiones inicial!", BR_NOMBRE_CHAT.c_str());
        break;
    }
    case BR_MENSAJE_ERROR_EN_COLA:
    {
        Chat(player).PSendSysMessage("%s ¡Ya estas en cola para el evento!", BR_NOMBRE_CHAT.c_str());
        break;
    }
    case BR_MENSAJE_ERROR_INVITADO:
    {
        Chat(player).PSendSysMessage("%s ¡Ya has sido invitado al evento en curso!", BR_NOMBRE_CHAT.c_str());
        break;
    }
    case BR_MENSAJE_ERROR_EN_EVENTO:
    {
        Chat(player).PSendSysMessage("%s ¡Ya estas dentro del evento!", BR_NOMBRE_CHAT.c_str());
        break;
    }
    }
}