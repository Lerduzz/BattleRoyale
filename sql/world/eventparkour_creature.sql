-- --------------------------------------------------------------------------------------
--	PARKOUR MANAGER - 190011
-- --------------------------------------------------------------------------------------
SET
@Entry 		:= 190011,
@Model 		:= 9353, -- Undead Necromancer
@Name 		:= "Gestor de Eventos John",
@Title 		:= "Parkour de la Muerte",
@Icon 		:= "Speak",
@GossipMenu := 0,
@MinLevel 	:= 80,
@MaxLevel 	:= 80,
@Faction 	:= 35,
@NPCFlag 	:= 1,
@Scale		:= 1.0,
@Rank		:= 0,
@Type 		:= 7,
@TypeFlags 	:= 0,
@FlagsExtra := 2,
@AIName		:= "SmartAI",
@Script 	:= "npc_eventparkour";

-- NPC
DELETE FROM creature_template WHERE entry = @Entry;
INSERT INTO creature_template (entry, modelid1, name, subname, IconName, gossip_menu_id, minlevel, maxlevel, faction, npcflag, speed_walk, speed_run, scale, `rank`, unit_class, unit_flags, type, type_flags, InhabitType, RegenHealth, flags_extra, AiName, ScriptName) VALUES
(@Entry, @Model, @Name, @Title, @Icon, @GossipMenu, @MinLevel, @MaxLevel, @Faction, @NPCFlag, 1, 1.14286, @Scale, @Rank, 1, 2, @Type, @TypeFlags, 3, 1, @FlagsExtra, @AIName, @Script);

-- NPC EQUIPPED
DELETE FROM `creature_equip_template` WHERE `CreatureID`=@Entry AND `ID`=1;
INSERT INTO `creature_equip_template` VALUES (@Entry, 1, 11343, 0, 0, 18019); -- Black/Purple Staff, None

-- NPC TEXT
DELETE FROM `npc_text` WHERE `ID`=@Entry;
INSERT INTO `npc_text` (`ID`, `text0_0`) VALUES (@Entry, 'Hola $N. Estoy a tu servicio.\r\nPuedo teletransportarte al evento Parkour de la Muerte.\r\n\r\nComo funciona el evento:\r\nAl seleccionar la opcion: Quiero ir al evento; eres teletransportado a la Arena de Gurubashi en la Vega de Tuercespina, pero en otra fase visible, o sea, que solo los jugadores en el evento pueden verse entre si.\r\nEl evento consiste en intentar ser el primero en llegar a la plataforma superior en una batalla de todos contra todos donde se puede usar cualquier habilidad de tu personaje y los recursos colocados en el evento.');
