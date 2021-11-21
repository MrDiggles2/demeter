
--------------------------------------------------------------------------------
-- Up
--------------------------------------------------------------------------------

-- Take a backup of Reading for reverting this migration
CREATE TABLE Reading_backup_002_for_revert (
    id   INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    raw INTEGER NOT NULL,
    addedAt INTEGER NOT NULL
);
INSERT INTO Reading_backup_002_for_revert SELECT id, name, raw, addedAt FROM Reading;

-- Add new column and table
ALTER TABLE Reading ADD COLUMN sensorId INTEGER;

CREATE TABLE Sensor (
    id  INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    ignore INTEGER NOT NULL
);

-- Populate table with unique sensor names
INSERT INTO SENSOR (name, ignore) SELECT DISTINCT(name) AS name, 0 as ignore FROM Reading;

-- Add the sensor IDs to the table based on name
UPDATE Reading
SET (sensorId) = (Sensor.id)
FROM Sensor
WHERE Reading.name = Sensor.name;

-- Create temp backup of Reading
CREATE TABLE Reading_backup_002_temp (
    id   INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    raw INTEGER NOT NULL,
    addedAt INTEGER NOT NULL,
    sensorId INTEGER NOT NULL
);
INSERT INTO Reading_backup_002_temp SELECT id, name, raw, addedAt, sensorId FROM Reading;

-- Recreate table without name column, copying in data from backup
DROP TABLE Reading;
CREATE TABLE Reading (
    id   INTEGER PRIMARY KEY,
    sensorId INTEGER NOT NULL,
    raw INTEGER NOT NULL,
    addedAt INTEGER NOT NULL
);
INSERT INTO Reading SELECT id, sensorId, raw, addedAt FROM Reading_backup_002_temp;
DROP TABLE Reading_backup_002_temp;

--------------------------------------------------------------------------------
-- Down
--------------------------------------------------------------------------------

-- Not worrying about data loss
DROP TABLE Reading;
ALTER TABLE Reading_backup_002_for_revert RENAME TO Reading;

DROP TABLE Sensor;