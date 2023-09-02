SET
@Entry 		:= 500000,
@DisplayID  := 8612, -- 7203
@Name       := 'Zona Segura BR';
DELETE FROM `gameobject_template` WHERE `entry` >= @Entry+0 AND `entry` <= @Entry+10;
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+0, @DisplayID, @Name, 4.5,  5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+1, @DisplayID, @Name, 4.0,  5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+2, @DisplayID, @Name, 3.5,  5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+3, @DisplayID, @Name, 3.0,  5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+4, @DisplayID, @Name, 2.5,  5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+5, @DisplayID, @Name, 2.0,  5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+6, @DisplayID, @Name, 1.5,  5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+7, @DisplayID, @Name, 1.0,  5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+8, @DisplayID, @Name, 0.5,  5, 1);
INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+9, @DisplayID, @Name, 0.25, 5, 1);

INSERT INTO `gameobject_template` (`entry`, `displayId`, `name`, `size`, `type`, `Data3`) VALUES (@Entry+10, 0, "Centro Invisible BR", 1.0, 5, 1);