SET
@Entry 		:= 200000,
@Model 		:= 19301,
@Name 		:= "Lerduzz el Comandante Supremo",
@Title 		:= "Gran Maestro del Battle Royale",
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
INSERT INTO `npc_text` (`ID`, `text0_0`) VALUES (@Entry, 'Saludos $N, como creador del evento Battle Royale de S King Inc me encargo de gestionar la cola de los participantes.\r\n\r\n¿De que va esto?\r\nEl evento tiene lugar en una zona aleatoria del mundo preparada previamente, los jugadores lucharan por adentrarse en la zona segura mientras intentan sobrevivir en una batalla todos contra todos.\r\n\r\nPara más información visita mi canal de YouTube:\r\nhttps://www.youtube.com/@lerduzz');
