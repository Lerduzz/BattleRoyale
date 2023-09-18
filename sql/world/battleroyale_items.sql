SET
@Entry 		   := 17,
@Class         := 4,
@Name          := 'Alas de Battle Royale',
@DisplayID     := 7016,
@Quality       := 6,
@ItemLevel     := 1,
@InventoryType := 4,
@Delay         := 0,
@Spell         := 18282,
@Cooldown       := 2500,
@Bonding       := 1,
@Description   := 'Activa y desactiva las Alas Mágicas del modo Battle Royale. ¡A que mola, Eh!',
@Material      := 6,
@Sheath        := 3,
@Duration      := 900,
@Flags         := 128,
@FlagsCustom   := 1,
@Script        := 'BattleRoyaleItem';

DELETE FROM `item_template` WHERE `entry` = @Entry;
DELETE FROM `item_template_locale` WHERE `ID` = @Entry;

INSERT INTO `item_template` (`entry`, `class`, `name`, `displayid`, `Quality`, `ItemLevel`, `InventoryType`, `delay`, `spellid_1`, `spellcooldown_1`, `bonding`, `description`, `Material`, `sheath`, `duration`, `Flags`, `flagsCustom`, `ScriptName`) VALUES
(@Entry, @Class, @Name, @DisplayID, @Quality, @ItemLevel, @InventoryType, @Delay, @Spell, @Cooldown, @Bonding, @Description, @Material, @Sheath, @Duration, @Flags, @FlagsCustom, @Script);