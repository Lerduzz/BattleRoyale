-- NPC de anotarse --
SET
@Entry 		:= 200000,
@Model1 	:= 23240,
@Name1 		:= "Lerduzz el Comandante",
@Title1 	:= "Administrador del Battle Royale",
@Icon 		:= "Speak",
@GossipMenu := 0,
@MinLevel 	:= 80,
@MaxLevel 	:= 80,
@Faction 	:= 35,
@NPCFlag1 	:= 1,
@Scale1		:= 0.6,
@Rank		:= 0,
@Type 		:= 4,
@TypeFlags 	:= 0,
@FlagsExtra := 2,
@AIName		:= "SmartAI",
@Script 	:= "BattleRoyaleCreature";

DELETE FROM `creature_template` WHERE `entry` = @Entry;
INSERT INTO `creature_template` (`entry`, `modelid1`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `unit_class`, `unit_flags`, `type`, `type_flags`, `RegenHealth`, `flags_extra`, `AiName`, `ScriptName`) VALUES
(@Entry, @Model1, @Name1, @Title1, @Icon, @GossipMenu, @MinLevel, @MaxLevel, @Faction, @NPCFlag1, 1, 1.14286, @Scale1, @Rank, 1, 2, @Type, @TypeFlags, 1, @FlagsExtra, @AIName, @Script);

DELETE FROM `npc_text` WHERE `ID` = @Entry;
INSERT INTO `npc_text` (`ID`, `text0_0`) VALUES (@Entry, 'Saludos $N, como creador del evento Battle Royale de S King Inc me encargo de gestionar la cola de los participantes.\r\n\r\n¿De que va esto?\r\nEl evento tiene lugar en una zona aleatoria del mundo preparada previamente, los jugadores lucharan por adentrarse en la zona segura mientras intentan sobrevivir en una batalla todos contra todos.\r\n\r\nPara más información visita mi canal de YouTube:\r\nhttps://www.youtube.com/@lerduzz');


-- NPC de las misiones --
SET
@Model2     := 19301,
@Name2      := "Skxawng el Instructor",
@Title2     := "Misiones del Battle Royale",
@NPCFlag2   := 2,
@Scale2     := 0.8;
DELETE FROM `creature_template` WHERE `entry` = @Entry + 1;
INSERT INTO `creature_template` (`entry`, `modelid1`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `unit_class`, `unit_flags`, `type`, `type_flags`, `RegenHealth`, `flags_extra`, `AiName`, `ScriptName`) VALUES
(@Entry + 1, @Model2, @Name2, @Title2, '', @GossipMenu, @MinLevel, @MaxLevel, @Faction, @NPCFlag2, 1, 1.14286, @Scale2, @Rank, 1, 2, @Type, @TypeFlags, 1, @FlagsExtra, '', '');

-- Mision 1 (Diaria): Participa en 10 rondas de Battle Royale --


-- Mision 2 (Diaria): Elimina a 5 jugadores en Battle Royale --
-- Mision 3 (Diaria): En una ronda de Battle Royale termina en el TOP 5 eliminando al menos a 2 jugadores --
-- Mision 4 (Semanal): Gana 3 rondas de Battle Royale --
-- Mision 5 (Semanal): Gana 1 rondas de Battle Royale eliminando almenos a 3 jugadores --
