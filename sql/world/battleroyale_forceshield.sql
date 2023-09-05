SET
@Entry 		:= 500000,
@DisplayID  := 8612,
@Name       := 'Zona Segura BR';
DELETE FROM `gameobject_template` WHERE `entry` >= @Entry+0 AND `entry` < @Entry+11;
DELETE FROM `gameobject_template_addon` WHERE `entry` >= @Entry+0 AND `entry` < @Entry+11;

-- Centro de la partida: se puede intentar poner un AZ de luz.
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`) VALUES (@Entry+0, 0, "Centro Invisible BR", 1.0, 5);
INSERT INTO `gameobject_template_addon` (`entry`, `flags`) VALUES (@Entry+0, 32);

INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+1,  @DisplayID, @Name, 6.0, 5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+2,  @DisplayID, @Name, 5.5, 5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+3,  @DisplayID, @Name, 5.0, 5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+4,  @DisplayID, @Name, 4.5, 5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+5,  @DisplayID, @Name, 4.0, 5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+6,  @DisplayID, @Name, 3.5, 5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+7,  @DisplayID, @Name, 3.0, 5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+8,  @DisplayID, @Name, 2.5, 5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+9,  @DisplayID, @Name, 2.0, 5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+10, @DisplayID, @Name, 1.5, 5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+11, @DisplayID, @Name, 1.0, 5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+12, @DisplayID, @Name, 0.5, 5, 1);

INSERT INTO `gameobject_template_addon` (`entry`, `flags`) VALUES (@Entry+1,  32);
INSERT INTO `gameobject_template_addon` (`entry`, `flags`) VALUES (@Entry+2,  32);
INSERT INTO `gameobject_template_addon` (`entry`, `flags`) VALUES (@Entry+3,  32);
INSERT INTO `gameobject_template_addon` (`entry`, `flags`) VALUES (@Entry+4,  32);
INSERT INTO `gameobject_template_addon` (`entry`, `flags`) VALUES (@Entry+5,  32);
INSERT INTO `gameobject_template_addon` (`entry`, `flags`) VALUES (@Entry+6,  32);
INSERT INTO `gameobject_template_addon` (`entry`, `flags`) VALUES (@Entry+7,  32);
INSERT INTO `gameobject_template_addon` (`entry`, `flags`) VALUES (@Entry+8,  32);
INSERT INTO `gameobject_template_addon` (`entry`, `flags`) VALUES (@Entry+9,  32);
INSERT INTO `gameobject_template_addon` (`entry`, `flags`) VALUES (@Entry+10, 32);
INSERT INTO `gameobject_template_addon` (`entry`, `flags`) VALUES (@Entry+11, 32);
INSERT INTO `gameobject_template_addon` (`entry`, `flags`) VALUES (@Entry+12, 32);
