ALTER TABLE `dbc_spell` ADD `Attributes`    INT(11) UNSIGNED DEFAULT NULL NULL AFTER `EffectBasePoints2` ;
ALTER TABLE `dbc_spell` ADD `AttributesEx`  INT(11) UNSIGNED DEFAULT NULL NULL AFTER `Attributes` ;
ALTER TABLE `dbc_spell` ADD `AttributesEx2` INT(11) UNSIGNED DEFAULT NULL NULL AFTER `AttributesEx` ;
ALTER TABLE `dbc_spell` ADD `AttributesEx3` INT(11) UNSIGNED DEFAULT NULL NULL AFTER `AttributesEx2` ;
ALTER TABLE `dbc_spell` ADD `AttributesEx4` INT(11) UNSIGNED DEFAULT NULL NULL AFTER `AttributesEx3` ;
ALTER TABLE `dbc_spell` ADD `AttributesEx5` INT(11) UNSIGNED DEFAULT NULL NULL AFTER `AttributesEx4` ;
ALTER TABLE `dbc_spell` ADD `AttributesEx6` INT(11) UNSIGNED DEFAULT NULL NULL AFTER `AttributesEx5` ;

REPLACE INTO `dbc_spell` (`SpellID`, `CastingTimeIndex`, `DurationIndex`, `EffectBasePoints0`, `EffectBasePoints1`, `EffectBasePoints2`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `AttributesEx4`, `AttributesEx5`, `AttributesEx6`, `Comment`) VALUES ('3600', NULL, NULL, NULL, NULL, NULL, NULL, '168', NULL, NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `dbc_spell` (`SpellID`, `CastingTimeIndex`, `DurationIndex`, `EffectBasePoints0`, `EffectBasePoints1`, `EffectBasePoints2`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `AttributesEx4`, `AttributesEx5`, `AttributesEx6`, `Comment`) VALUES ('29858', NULL, NULL, NULL, NULL, NULL, NULL, '1056', NULL, NULL, NULL, NULL, NULL, NULL);
