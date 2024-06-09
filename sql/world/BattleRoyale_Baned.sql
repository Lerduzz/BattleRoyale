CREATE TABLE IF NOT EXISTS `battleroyale_blacklist` ( 
    `guid` INT UNSIGNED NOT NULL,
    `reason` VARCHAR(255) NOT NULL DEFAULT 'Sin motivo.',
PRIMARY KEY (`guid`) );