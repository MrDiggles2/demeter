--------------------------------------------------------------------------------
-- Up
--------------------------------------------------------------------------------

CREATE TABLE Reading (
    id   INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    raw INTEGER NOT NULL,
    percent INTEGER NOT NULL,
    max INTEGER NOT NULL,
    min INTEGER NOT NULL,
    addedAt INTEGER NOT NULL
);

INSERT INTO Reading (
    name,
    raw,
    percent,
    max,
    min,
    addedAt
) VALUES (
    'Test',
    34,
    34,
    100,
    0,
    1604825392
);

--------------------------------------------------------------------------------
-- Down
--------------------------------------------------------------------------------

DROP TABLE Reading;