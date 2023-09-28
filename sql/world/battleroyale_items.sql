SET
@Quality       := 6,
@Duration      := 900;

DELETE FROM `item_template` WHERE `entry` IN (17, 20902);
DELETE FROM `item_template_locale` WHERE `ID` IN (17, 20902);

INSERT INTO `item_template`
(`entry`, `class`, `subclass`, `name`,                        `displayid`, `Quality`, `ItemLevel`, `InventoryType`, `delay`, `spellid_1`, `spellcooldown_1`, `bonding`, `description`,                                                                        `Material`, `duration`, `Flags`, `flagsCustom`, `ScriptName`) VALUES
(17,      4,       0,          'Camisa de Battle Royale',     7016,        @Quality,  1,           4,               0,       18282,       1500,              1,         'Uso: Activa y desactiva las Alas Mágicas del modo Battle Royale cada 1.5 segundos.', 6,          @Duration,  128,     1,             'BattleRoyaleItem'),
(20902,   4,       1,          'Pantalones de Battle Royale', 33359,       @Quality,  1,           7,               0,       0,           -1,                1,         'Pantalones de inicio del Battle Royale, para proteger tus partes del frío.',         7,          @Duration,  0,       1,             '');
