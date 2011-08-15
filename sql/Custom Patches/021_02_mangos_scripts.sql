/* SUNWELL PLATEAU */
UPDATE instance_template SET ScriptName='instance_sunwell_plateau' WHERE map=580;
UPDATE creature_template SET ScriptName='boss_brutallus' WHERE entry=24882;
UPDATE creature_template SET ScriptName='boss_kalecgos' WHERE entry=24850;
UPDATE creature_template SET ScriptName='boss_kalecgos_humanoid' WHERE entry=24891;
UPDATE creature_template SET ScriptName='boss_sathrovarr' WHERE entry=24892;
UPDATE gameobject_template SET ScriptName='go_spectral_rift' WHERE entry=187055;

DELETE FROM scripted_areatrigger WHERE entry=4853;
INSERT INTO scripted_areatrigger VALUES (4853,'at_madrigosa');

DELETE FROM scripted_areatrigger WHERE entry=4937;
INSERT INTO scripted_areatrigger VALUES (4937, 'at_sunwell_plateau');

UPDATE creature_template SET ScriptName='boss_alythess' WHERE entry=25166;
UPDATE creature_template SET ScriptName='boss_sacrolash' WHERE entry=25165;
UPDATE creature_template SET ScriptName='npc_shadow_image' WHERE entry=25214;

