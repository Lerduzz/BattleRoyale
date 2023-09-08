SET
@Entry 		:= 499999,
@Type       := 3,
@DisplayID  := 259,
@Name       := 'Cofre de Battle Royale',
@OpenLock   := 1599,
@LootID     := 200000,
@Consumable := 1;

DELETE FROM `gameobject_template` WHERE `entry` = @Entry;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `Data0`, `Data1`, `Data3`) VALUES (@Entry, @Type, @DisplayID, @Name, @OpenLock, @LootID, @Consumable);

DELETE FROM `gameobject_loot_template` WHERE `Entry` = @LootID;
INSERT INTO `gameobject_loot_template` (`Entry`, `Item`, `Chance`, `MinCount`, `MaxCount`, `Comment`) VALUES (@LootID, 33447, 100, 1, 3, 'Cofre BR - Posion de sanacion');
INSERT INTO `gameobject_loot_template` (`Entry`, `Item`, `Chance`, `MinCount`, `MaxCount`, `Comment`) VALUES (@LootID, 33448,  80, 1, 3, 'Cofre BR - Posion de mana');
INSERT INTO `gameobject_loot_template` (`Entry`, `Item`, `Chance`, `MinCount`, `MaxCount`, `Comment`) VALUES (@LootID, 40211,  10, 1, 2, 'Cofre BR - Posion de velocidad');
INSERT INTO `gameobject_loot_template` (`Entry`, `Item`, `Chance`, `MinCount`, `MaxCount`, `Comment`) VALUES (@LootID, 40093,  10, 1, 2, 'Cofre BR - Posion de indestructible');
INSERT INTO `gameobject_loot_template` (`Entry`, `Item`, `Chance`, `MinCount`, `MaxCount`, `Comment`) VALUES (@LootID, 40087,   5, 1, 1, 'Cofre BR - Posion de rejuvenecimiento');

-- Graveyard fix.
-- INSERT INTO `game_graveyard` (`ID`, `Map`, `x`, `y`, `z`, `Comment`) VALUES ('2000', '1', '5261.581055', '-2164.183105', '1259.483765', 'Hyjal - BR - Test'); 
-- INSERT INTO `acore_world`.`graveyard_zone` (`ID`, `GhostZone`, `Comment`) VALUES ('2000', '616', 'Hyjal - BR - Test 0'); 
