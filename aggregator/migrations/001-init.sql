--------------------------------------------------------------------------------
-- Up
--------------------------------------------------------------------------------

CREATE TABLE Reading (
    id   INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    raw INTEGER NOT NULL,
    addedAt INTEGER NOT NULL
);

--------------------------------------------------------------------------------
-- Down
--------------------------------------------------------------------------------

DROP TABLE Reading;