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

-- Filter out short slices and NULLs
CREATE VIEW test_and_power_deltas_filtered AS
SELECT
 *
FROM test_and_power_deltas
WHERE dur > 240000000
  AND delta IS NOT NULL
ORDER BY ts ASC;


CREATE VIEW nanobench_powers AS
SELECT name as test_name, counter_name as rail_name, SUM(delta/(dur/1000000)) AS power_mws
FROM test_and_power_deltas_filtered
GROUP BY name, counter_name;

--SELECT * from nanobench_power;

CREATE VIEW nanobench_power_output AS
SELECT NanobenchResults(
  'nanobench_result', (
    SELECT RepeatedField(
      NanobenchResult(
        'test_name', test_name,
        'rail_name', rail_name,
        'power_mws', power_mws
      )
    )
    FROM nanobench_powers
  )
);