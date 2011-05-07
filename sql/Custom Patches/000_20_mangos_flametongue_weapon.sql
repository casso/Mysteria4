-- Update Elemental Weapons-Talent to support Flametongue Weapon
UPDATE spell_affect SET SpellFamilyMask = SpellFamilyMask | 0x200000 WHERE effectid = 1 AND entry IN (16266, 29079, 29080);

-- Custom spell_proc_event for Flametongue Weapon
INSERT INTO spell_proc_event VALUES
(33756, 0, 0, 0, 0, 0, 0, 100, 0);
