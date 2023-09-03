SET
@Entry 		:= 200000,
@Model 		:= 9353,
@Name 		:= "Maestro del Battle Royale",
@Title 		:= "Battle Royale en S King Inc",
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
@Script 	:= "npc_battleroyale";

-- NPC
DELETE FROM creature_template WHERE entry = @Entry;
INSERT INTO creature_template (`entry`, `modelid1`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `unit_class`, `unit_flags`, `type`, `type_flags`, `RegenHealth`, `flags_extra`, `AiName`, `ScriptName`) VALUES
(@Entry, @Model, @Name, @Title, @Icon, @GossipMenu, @MinLevel, @MaxLevel, @Faction, @NPCFlag, 1, 1.14286, @Scale, @Rank, 1, 2, @Type, @TypeFlags, 1, @FlagsExtra, @AIName, @Script);

-- NPC TEXT
DELETE FROM `npc_text` WHERE `ID` = @Entry;
INSERT INTO `npc_text` (`ID`, `text0_0`) VALUES (@Entry, 'Hola $N. Estoy a tu servicio.\r\nMe encargo de gestionar la cola para el evento Battle Royale de S King Inc.\r\n\r\n¿De que va esto?\r\nEl evento se efectua en una zona aleatoria del mundo (pero solo los participantes pueden verse entre si), los jugadores lucharan por adentrarse en la zona segura mientras intentan sobrevivir en una batalla todos contra todos.');

-- WAYPOINTS
SET 
@WPPath    := 300000,
@WPComment := 'Vuelo de Battle Royale';
DELETE FROM `waypoints` WHERE `entry` >= @WPPath AND `entry` < @WPPath + 1;
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `point_comment`) VALUES 
(@WPPath, 1, 5411.041504, -2701.873779, 1451.660522, @WPComment),
(@WPPath, 2, 5261.581055, -2164.183105, 1559.483765, @WPComment),
(@WPPath, 3, 5261.581055, -2164.183105, 1599.483765, @WPComment);

-- SMART SCRIPTS
DELETE FROM `smart_scripts` WHERE `entryorguid` = @WPPath AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
-- ENTRY  ST   ID   LINK ETYPE PHASE CHANCE FLAGS  EP1    EP2     EP3  EP4  EP5   ATYPE AP1      AP2      AP3  AP4      AP5   AP6  TTYPE TP1 TP2  TP3  TP4  X    Y    Z    O    COMMENT
(@WPPath, 0,   0,   '0', '27', '0',  '100', '512', '0',  '0',     '0', '0', '0',  '53', '0',     '33519', '0', '13663', '0', '0',  '1', '0', '0', '0', '0', '0', '0', '0', '0', 'Vuelo de BR - On passenger - Start WP movement'),
(@WPPath, 0,   1,   '0', '40', '0',  '100', '512', '2',  '33519', '0', '0', '0',  '11', '50630', '0',     '0', '0',     '0', '0',  '0', '0', '0', '0', '0', '0', '0', '0', '0', 'Vuelo de BR - On WP 2 - Dismount Spell'),
(@WPPath, 0,   2,   '0', '40', '0',  '100', '512', '3',  '33519', '0', '0', '0',  '41', '0',     '0',     '0', '0',     '0', '0',  '1', '0', '0', '0', '0', '0', '0', '0', '0', 'Vuelo de BR - On WP 3 - Despawn');
