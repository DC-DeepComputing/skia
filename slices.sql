SELECT name, ts AS ts_begin, ts+dur AS ts_end
FROM slice
WHERE name LIKE "GM_%";
