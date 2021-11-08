SELECT counter_track.name AS counter_name , counter.ts AS ts, counter.value AS value
FROM counter
JOIN counter_track ON counter_track.id = counter.track_id
WHERE counter_track.name like 'power.%'