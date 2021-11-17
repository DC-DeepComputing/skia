-- Convert the power measurements into slices.
CREATE VIEW power_slices AS
SELECT
  counter_track.name AS rail_name,
  counter.ts AS ts,
  counter.value AS value,
  LEAD(ts) OVER (
    PARTITION BY track_id
    ORDER BY
      ts
  ) - ts as dur
FROM
  counter
  JOIN counter_track ON counter_track.id = counter.track_id
WHERE
  counter_track.name like 'power.%';

-- Create a view with only the test slices.
CREATE VIEW test_slices AS
SELECT
  ts,
  dur,
  (
    select
      string_value
    from
      args
    where
      key = 'debug.name'
      AND slice.arg_set_id == args.arg_set_id
  ) as test_name,
  (
    select
      string_value
    from
      args
    where
      key = 'debug.config'
      AND slice.arg_set_id == args.arg_set_id
  ) as config_name,
  (
    select
      int_value
    from
      args
    where
      key = 'debug.loops'
      AND slice.arg_set_id == args.arg_set_id
  ) as loops
FROM
  slice
WHERE
  category = 'skia'
  AND name = 'nanobench';

-- SPAN_JOIN the test and power slices together.
CREATE VIRTUAL TABLE test_and_power_slices USING SPAN_JOIN(power_slices, test_slices);

CREATE VIEW test_and_power_deltas AS
SELECT
  ts,
  dur,
  test_name,
  config_name,
  loops,
  rail_name,
  (
    LEAD(value) OVER (
      PARTITION BY rail_name,
      test_name,
      config_name
      ORDER BY
        ts
    ) - value
  ) AS delta
FROM
  test_and_power_slices
ORDER BY
  ts ASC;

-- Filter out short slices and NULLs
CREATE VIEW test_and_power_deltas_filtered AS
SELECT
  *
FROM
  test_and_power_deltas
WHERE
  dur > 240000000
  AND delta IS NOT NULL
ORDER BY
  ts ASC;

-- Calculate the total power draw in mW/s for each test and rail.
CREATE VIEW nanobench_powers AS
SELECT
  test_name,
  config_name,
  rail_name,
  loops,
  SUM((delta /(dur / 1000000))) AS total_power,
  SUM((delta /(dur / 1000000)) / loops) AS power_mws
FROM
  test_and_power_deltas_filtered
GROUP BY
  test_name,
  config_name,
  rail_name;

-- Export the view as a proto.
CREATE VIEW nanobench_power_output AS
SELECT
  NanobenchResults(
    'nanobench_result',
    (
      SELECT
        RepeatedField(
          NanobenchResult(
            'test_name',
            test_name,
            'config_name',
            config_name,
            'rail_name',
            rail_name,
            'loops',
            loops,
            'power_mws',
            power_mws,
            'total_power',
            total_power
          )
        )
      FROM
        nanobench_powers
    )
  );