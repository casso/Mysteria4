CREATE TABLE `character_login_history` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `id_character` int(11) unsigned NOT NULL,
  `id_account` int(11) unsigned NOT NULL,
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
