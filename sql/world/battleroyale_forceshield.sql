SET
@Entry 		:= 500000,
@DisplayID  := 7203,
@Name       := 'Zona Segura BR';
DELETE FROM `gameobject_template` WHERE `entry` >= @Entry+0 AND `entry` <= @Entry+9;
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`) VALUES (@Entry+0, @DisplayID, @Name, 1.8);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`) VALUES (@Entry+1, @DisplayID, @Name, 1.6);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`) VALUES (@Entry+2, @DisplayID, @Name, 1.4);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`) VALUES (@Entry+3, @DisplayID, @Name, 1.2);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`) VALUES (@Entry+4, @DisplayID, @Name, 1.0);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`) VALUES (@Entry+5, @DisplayID, @Name, 0.8);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`) VALUES (@Entry+6, @DisplayID, @Name, 0.6);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`) VALUES (@Entry+7, @DisplayID, @Name, 0.4);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`) VALUES (@Entry+8, @DisplayID, @Name, 0.2);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`) VALUES (@Entry+9, @DisplayID, @Name, 0.05);