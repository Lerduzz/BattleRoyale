DROP TABLE IF EXISTS `battleroyale_maps`;
CREATE TABLE `battleroyale_maps` (
    `id` INT NOT NULL AUTO_INCREMENT, 
    `map_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `map_name` VARCHAR(255) NOT NULL,
    `center_x` FLOAT NOT NULL DEFAULT 0,
    `center_y` FLOAT NOT NULL DEFAULT 0,
    `center_z` FLOAT NOT NULL DEFAULT 0,
    `center_o` FLOAT NOT NULL DEFAULT 0,
    `ship_x` FLOAT NOT NULL DEFAULT 0,
    `ship_y` FLOAT NOT NULL DEFAULT 0,
    `ship_z` FLOAT NOT NULL DEFAULT 0,
    `ship_o` FLOAT NOT NULL DEFAULT 0,
PRIMARY KEY (`id`));

DROP TABLE IF EXISTS `battleroyale_maps_spawns`;
CREATE TABLE `battleroyale_maps_spawns` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `zone` INT NOT NULL DEFAULT 0,
    `map` INT NOT NULL DEFAULT 0,
    `pos_x` FLOAT NOT NULL DEFAULT 0,
    `pos_y` FLOAT NOT NULL DEFAULT 0,
    `pos_z` FLOAT NOT NULL DEFAULT 0,
    `pos_o` FLOAT NOT NULL DEFAULT 0,
PRIMARY KEY (`id`));

INSERT  INTO `battleroyale_maps`(`id`,`map_id`,`map_name`,`center_x`,`center_y`,`center_z`,`center_o`,`ship_x`,`ship_y`,`ship_z`,`ship_o`) VALUES
(1, 1, 'Kalimdor: Hyjal', 5261.58, -2164.18, 1259.48, 0, 2967.58, -2164.18, 1556.48, -1.5708),
(2, 0, 'Reinos del Este: Isla Jaguero', -14614.6, -313.263, 0.000001, 0, -12320.6, -313.263, 297, 1.5708),
(3, 1, 'Kalimdor: Las Charcas del Olvido', 157.996, -1948.09, 87.3871, 0, -2137.58, -1948.09, 394.387, -1.5708),
(4, 1, 'Kalimdor: Ruinas de Solarsal', -4695.31, 3411.21, 7.05051, 0, -2401.31, 3411.21, 304.051, 1.5708);

INSERT INTO `battleroyale_maps_spawns`(`id`,`zone`,`map`,`pos_x`,`pos_y`,`pos_z`,`pos_o`) VALUES
(1, 0, 1, 5137.57, -1903.26, 1364.18, 5.19384),
(2, 0, 1, 5284.84, -1878.79, 1353.59, 4.77363),
(3, 0, 1, 5013.89, -2037.03, 1366.05, 6.02243),
(4, 0, 1, 5126.78, -2385.4, 1421.12, 1.19616),
(5, 0, 1, 5445.92, -2295.33, 1458.18, 2.65464);