UPDATE characters SET at_login = at_login | '4' WHERE class = 11;
DELETE FROM character_spell WHERE spell IN (37716, 37117, 16958, 16961);
