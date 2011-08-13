CREATE TABLE `guildhouse_position` (
  `guildid` int(11) NOT NULL,
  `x` float NOT NULL,
  `y` float NOT NULL,
  `z` float NOT NULL,
  `o` float NOT NULL,
  `mapid` int(11) unsigned NOT NULL,
  `comment` varchar(256) DEFAULT NULL,
  PRIMARY KEY (`guildid`)
);
