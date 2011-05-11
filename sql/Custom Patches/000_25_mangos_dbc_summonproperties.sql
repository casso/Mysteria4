CREATE TABLE `dbc_summonproperties` (
  `id` int(11) unsigned NOT NULL,
  `group_property` int(11) unsigned DEFAULT NULL,
  `faction` int(11) unsigned DEFAULT NULL,
  `type` int(11) unsigned DEFAULT NULL,
  `slot` int(11) unsigned DEFAULT NULL,
  `flags` int(11) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

INSERT INTO `dbc_summonproperties` (`id`, `group_property`, `faction`, `type`, `slot`, `flags`) VALUES
	('407',NULL,'35',NULL,NULL,NULL);
