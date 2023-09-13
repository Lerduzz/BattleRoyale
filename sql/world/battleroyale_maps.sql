DROP TABLE IF EXISTS `battleroyale_maps`;
CREATE TABLE `battleroyale_maps` (
    `id` INT UNSIGNED NOT NULL AUTO_INCREMENT, 
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

