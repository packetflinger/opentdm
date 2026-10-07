-- Match stats POSTed by OpenTDM servers (g_send_stats / g_stats_url).
-- All times are UTC, stored as "YYYY-MM-DDTHH:MM:SSZ" so they sort as text.

CREATE TABLE IF NOT EXISTS matches (
    id            INTEGER PRIMARY KEY,
    time          TEXT    NOT NULL,  -- when the match ended, from the game server
    received      TEXT    NOT NULL,  -- when statsrv got it
    source        TEXT    NOT NULL,  -- address the POST came from
    map           TEXT,
    demo          TEXT,
    demo_hostname TEXT,
    home_team     TEXT,
    home_score    INTEGER NOT NULL,
    away_team     TEXT,
    away_score    INTEGER NOT NULL
);

CREATE INDEX IF NOT EXISTS matches_time ON matches (time, id);

CREATE TABLE IF NOT EXISTS players (
    id                   INTEGER PRIMARY KEY,
    match_id             INTEGER NOT NULL REFERENCES matches (id) ON DELETE CASCADE,
    name                 TEXT,
    stats_id             TEXT,
    team                 TEXT    NOT NULL,  -- "home" or "away"
    kills                INTEGER NOT NULL,
    deaths               INTEGER NOT NULL,
    suicides             INTEGER NOT NULL,
    team_kills           INTEGER NOT NULL,
    telefrags            INTEGER NOT NULL,
    damage_dealt         INTEGER NOT NULL,
    damage_received      INTEGER NOT NULL,
    team_damage_dealt    INTEGER NOT NULL,
    team_damage_received INTEGER NOT NULL
);

CREATE INDEX IF NOT EXISTS players_match ON players (match_id);
CREATE INDEX IF NOT EXISTS players_stats_id ON players (stats_id);

-- per player weapon and item stats, the "items" array of each player
CREATE TABLE IF NOT EXISTS player_items (
    id              INTEGER PRIMARY KEY,
    player_id       INTEGER NOT NULL REFERENCES players (id) ON DELETE CASCADE,
    name            TEXT,
    classname       TEXT,
    accuracy        REAL,  -- NULL for non-weapons and weapons never fired
    shots           INTEGER NOT NULL,
    hits            INTEGER NOT NULL,
    kills           INTEGER NOT NULL,
    deaths          INTEGER NOT NULL,
    damage_dealt    INTEGER NOT NULL,
    damage_received INTEGER NOT NULL,
    pickups         INTEGER NOT NULL,
    missed          INTEGER NOT NULL
);

CREATE INDEX IF NOT EXISTS player_items_player ON player_items (player_id);

CREATE TABLE IF NOT EXISTS spectators (
    id       INTEGER PRIMARY KEY,
    match_id INTEGER NOT NULL REFERENCES matches (id) ON DELETE CASCADE,
    name     TEXT,
    stats_id TEXT
);

CREATE INDEX IF NOT EXISTS spectators_match ON spectators (match_id);
