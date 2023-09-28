SET
@Quality       := 6,
@Duration      := 900;

DELETE FROM `item_template` WHERE `entry` IN (17);
DELETE FROM `item_template_locale` WHERE `ID` IN (17);

INSERT INTO `item_template` (`entry`, `class`, `name`, `displayid`, `Quality`, `ItemLevel`, `InventoryType`, `delay`, `spellid_1`, `spellcooldown_1`, `bonding`, `description`, `Material`, `sheath`, `duration`, `Flags`, `flagsCustom`, `ScriptName`) VALUES
(17, 4, 'Camisa de Battle Royale', 7016, @Quality, 1, 4, 0, 18282, 1500, 1, 'Uso: Activa y desactiva las Alas Mágicas del modo Battle Royale cada 1.5 segundos.', 6, 3, @Duration, 128, 1, 'BattleRoyaleItem');