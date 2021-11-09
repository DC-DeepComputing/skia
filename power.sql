CREATE VIEW power_slices AS
SELECT
  counter_track.name AS counter_name,
  counter.ts AS ts,
  counter.value AS value,
  lead(ts) OVER (PARTITION BY track_id ORDER BY ts) - ts as dur
FROM counter
JOIN counter_track ON counter_track.id = counter.track_id
WHERE counter_track.name like 'power.%';


CREATE VIEW test_slices AS
SELECT name, ts ,dur
FROM slice
WHERE name LIKE "GM_%";

CREATE VIRTUAL TABLE test_and_power_slices
USING SPAN_JOIN(test_slices, power_slices);


-- SELECT * from test_and_power_slices;

CREATE VIEW test_and_power_deltas AS
SELECT
 ts,
 dur,
 name,
 counter_name,
 value AS value,
 LEAD(value) OVER (PARTITION BY counter_name, name ORDER BY ts) AS next_value,
 (LEAD(value) OVER (PARTITION BY counter_name, name ORDER BY ts) - value) AS delta
FROM test_and_power_slices
ORDER BY ts ASC;

SELECT name, counter_name, SUM(delta/(dur/1000000)) from test_and_power_deltas
GROUP BY name, counter_name;

