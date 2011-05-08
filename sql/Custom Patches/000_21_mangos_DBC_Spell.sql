CREATE TABLE `dbc_spell` (
  `SpellID` int(9) unsigned NOT NULL,
  `CastingTimeIndex` int(9) unsigned DEFAULT NULL,
  `DurationIndex` int(9) unsigned DEFAULT NULL,
  `EffectBasePoints0` int(9) DEFAULT NULL,
  `EffectBasePoints1` int(9) DEFAULT NULL,
  `EffectBasePoints2` int(9) DEFAULT NULL,
  `Comment` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`SpellID`),
  UNIQUE KEY `SpellID` (`SpellID`),
  KEY `SpellID_2` (`SpellID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

REPLACE INTO `command` (`name`, `security`) VALUES ('reload dbc', '5') ;
