CREATE TABLE `account_login_history` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `id_account` int(11) unsigned NOT NULL,
  `ip_address` varchar(20) NOT NULL,
  `login_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
