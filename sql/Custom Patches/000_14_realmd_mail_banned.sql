CREATE TABLE `mail_banned` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `mail` text NOT NULL,
  PRIMARY KEY  (`id`)
) COMMENT='mail ban system';