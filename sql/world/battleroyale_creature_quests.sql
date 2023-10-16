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

DELETE FROM `creature_template` WHERE `entry` >= @Entry - 1 AND `entry` <= @Entry + 3;
INSERT INTO `creature_template` (`entry`, `modelid1`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `unit_class`, `unit_flags`, `type`, `type_flags`, `RegenHealth`, `flags_extra`, `AiName`, `ScriptName`) VALUES
(@Entry, @Model1, @Name1, @Title1, @Icon, @GossipMenu, @MinLevel, @MaxLevel, @Faction, @NPCFlag1, 1, 1.14286, @Scale1, @Rank, 1, 2, @Type, @TypeFlags, 1, @FlagsExtra, @AIName, @Script);

DELETE FROM `npc_text` WHERE `ID` = @Entry;
INSERT INTO `npc_text` (`ID`, `text0_0`) VALUES (@Entry, 'Saludos $N, como creador del evento Battle Royale de S King Inc me encargo de gestionar la cola de los participantes.\r\n\r\n¿De que va esto?\r\nEl evento tiene lugar en una zona aleatoria del mundo preparada previamente, los jugadores lucharan por adentrarse en la zona segura mientras intentan sobrevivir en una batalla todos contra todos.\r\n\r\nPara más información visita mi canal de YouTube:\r\nhttps://www.youtube.com/@lerduzz');


-- NPC vendedor de armas --
SET
@Model2     := 23334,
@Name2      := "Alythess la Bruja Suprema",
@Title2     := "Vendedora de armas de Battle Royale",
@NPCFlag2   := 4224,
@Scale2     := 0.7;
INSERT INTO `creature_template` (`entry`, `modelid1`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `unit_class`, `unit_flags`, `type`, `type_flags`, `RegenHealth`, `flags_extra`, `AiName`, `ScriptName`) VALUES
(@Entry + 1, @Model2, @Name2, @Title2, '', @GossipMenu, @MinLevel, @MaxLevel, @Faction, @NPCFlag2, 1, 1.14286, @Scale2, @Rank, 1, 2, 7, @TypeFlags, 1, @FlagsExtra, '', '');

DELETE FROM `npc_vendor` WHERE `entry` = @Entry + 1;
INSERT INTO `npc_vendor` (`entry`, `item`) VALUES
(@Entry + 1, 34985),
(@Entry + 1, 34987),
(@Entry + 1, 34988),
(@Entry + 1, 34989),
(@Entry + 1, 34995),
(@Entry + 1, 34996),
(@Entry + 1, 34997),
(@Entry + 1, 35014),
(@Entry + 1, 35015),
(@Entry + 1, 35017),
(@Entry + 1, 35018),
(@Entry + 1, 35037),
(@Entry + 1, 35038),
(@Entry + 1, 35047),
(@Entry + 1, 35058),
(@Entry + 1, 35064),
(@Entry + 1, 35065),
(@Entry + 1, 35071),
(@Entry + 1, 35072),
(@Entry + 1, 35075),
(@Entry + 1, 35076),
(@Entry + 1, 35082),
(@Entry + 1, 35093),
(@Entry + 1, 35095),
(@Entry + 1, 35101),
(@Entry + 1, 35102),
(@Entry + 1, 35103),
(@Entry + 1, 35107),
(@Entry + 1, 35108),
(@Entry + 1, 35109);

-- NPC de las misiones --
SET
@Model3     := 19301,
@Name3      := "Skxawng el Instructor",
@Title3     := "Misiones del Battle Royale",
@NPCFlag3   := 2,
@Scale3     := 0.8;
INSERT INTO `creature_template` (`entry`, `modelid1`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `unit_class`, `unit_flags`, `type`, `type_flags`, `RegenHealth`, `flags_extra`, `AiName`, `ScriptName`) VALUES
(@Entry + 2, @Model3, @Name3, @Title3, '', @GossipMenu, @MinLevel, @MaxLevel, @Faction, @NPCFlag3, 1, 1.14286, @Scale3, @Rank, 1, 2, @Type, @TypeFlags, 1, @FlagsExtra, '', ''),
(@Entry + 3, 134, 'Mision 1 Credit 1', 'Battle Royale', '', 0, @MinLevel, @MaxLevel, @Faction, 0, 1, 1.14286, 1, 0, 1, 2, @Type, @TypeFlags, 1, 0, '', '');

DELETE FROM `quest_greeting` WHERE `ID` IN (@Entry + 1, @Entry + 2);
INSERT INTO `quest_greeting` (`ID`, `GreetEmoteType`, `Greeting`) VALUES (@Entry + 2, 1, 'Yo me encargo de entregar y recompensar todas las misiones relacionadas con el modo Battle Royale.'); 

SET
@QuestID    := 100000;
DELETE FROM `quest_template` WHERE `ID` = @QuestID;
DELETE FROM `quest_offer_reward` WHERE `ID` = @QuestID;
DELETE FROM `quest_request_items` WHERE `ID` = @QuestID;
DELETE FROM `quest_template_addon` WHERE `ID` = @QuestID;
INSERT INTO `quest_template` (`ID`, `QuestLevel`, `MinLevel`, `QuestSortID`, `RewardMoney`, `RewardHonor`, `Flags`, `RewardItem1`, `RewardAmount1`, `LogTitle`, `LogDescription`, `QuestDescription`, `QuestCompletionLog`, `RequiredNpcOrGo1`, `RequiredNpcOrGoCount1`, `ObjectiveText1`, `ObjectiveText2`, `ObjectiveText3`, `ObjectiveText4`) VALUES
(@QuestID, -1, 80, -22, 1000000, 5000, 4096, 47241, 10, 'Mantener la costumbre', 'Participa en 10 rondas del modo Battle Royale.', 'El modo Battle Royale está pensado para que todos ganen. Esta es una prueba de ello. Solo debes participar en algunas rondas y ya estarás ganando recompensas.', 'QuestCompletion log test.', @Entry + 3, 10, 'Rondas de Battle Royale jugadas', '', '', ''); 

INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `RewardText`) VALUES
(@QuestID, 1, '¡Felicitaciones! Regresa mañana para repetir la misión y ganar más recompensas.'); 

INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `CompletionText`) VALUES
(@QuestID, 1, 1, '¡Qué! ¿Regresas tan pronto?'); 

INSERT INTO `quest_template_addon` (`ID`, `SpecialFlags`) VALUES
(@QuestID, 1);

DELETE FROM `creature_queststarter` WHERE `id` IN (@Entry + 1, @Entry + 2);
INSERT INTO `creature_queststarter` (`id`, `quest`) VALUES
(@Entry + 2, @QuestID);

DELETE FROM `creature_questender` WHERE `id` IN (@Entry + 1, @Entry + 2);
INSERT INTO `creature_questender` (`id`, `quest`) VALUES
(@Entry + 2, @QuestID);

-- Mision 2 (Diaria): Elimina a 5 jugadores en Battle Royale --
-- Mision 3 (Diaria): En una ronda de Battle Royale termina en el TOP 5 eliminando al menos a 2 jugadores --
-- Mision 4 (Semanal): Gana 3 rondas de Battle Royale --
-- Mision 5 (Semanal): Gana 1 rondas de Battle Royale eliminando almenos a 3 jugadores --

-- NPC Guardian de Zona --
SET
@Model4     := 22965,
@Name4      := 'Madrigosa la Indomable',
@Title4     := 'Guardiana de Battle Royale',
@Level4     := 83,
@Faction4   := 50,
@NPCFlag4   := 0,
@Scale4     := 1;
INSERT INTO `creature_template` (`entry`, `modelid1`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `unit_class`, `unit_flags`, `type`, `type_flags`, `RegenHealth`, `flags_extra`, `AiName`, `ScriptName`, `HealthModifier`, `ManaModifier`) VALUES
(@Entry - 1, @Model4, @Name4, @Title4, '', @GossipMenu, @Level4, @Level4, @Faction4, @NPCFlag4, 1, 1.14286, @Scale4, 3, 8, 2+4+4096+131072+2147483648, 2, 4+16, 1, 256+16777216, '', '', 1780.5, 158.7);
DELETE FROM `creature_template_addon` WHERE `entry` = @Entry - 1;
INSERT INTO `creature_template_addon` (`entry`, `visibilityDistanceType`) VALUES (@Entry - 1, 5);
DELETE FROM `creature_template_movement` WHERE `CreatureId` = @Entry - 1;
INSERT INTO `creature_template_movement` (`CreatureId`, `Ground`, `Swim`, `Flight`, `Rooted`, `Chase`, `Random`, `InteractionPauseTimer`) VALUES (@Entry - 1, 1, 0, 1, 0, 1, 1, 60000); 
