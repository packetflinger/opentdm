# statsrv

A small HTTP server that collects match stats from OpenTDM game servers and
serves them back as JSON.

## Why

When a match ends, OpenTDM can POST a JSON summary of the match: the map,
teams and scores, every player's kills, deaths and damage, per-weapon
accuracy, item pickups, and the spectators. It does this only when
`g_send_stats` is enabled. The game server just sends the document and
forgets about it, so something has to receive it and keep it.

statsrv is that receiver. It stores each match in a SQLite database and lets
websites, bots and scripts fetch recent matches by date, without needing
access to the game server or the database file.

## Building

You need Go (see `go.mod` for the version) and, to create the database ahead
of time, the `sqlite3` command line tool. The SQLite driver is pure Go, so no C
compiler is needed and the result is a single static binary.

```sh
make            # builds ./statsrv and creates ./statsrv.db
make statsrv    # only build the binary
make db         # only create the database from schema.sql
make clean      # remove the binary (the database is left alone)
```

`make db` never touches a database that already exists. To create one
somewhere else:

```sh
make db DB=/var/lib/statsrv/stats.db
```

Creating the database first is optional. statsrv creates any missing tables
from `schema.sql` (built into the binary) every time it starts.

## Running

```sh
./statsrv [flags]
```

| Flag       | Default       | Description                                  |
|------------|---------------|----------------------------------------------|
| `-addr`    | `:47910`      | Address and port to listen on                |
| `-db`      | `statsrv.db`  | SQLite database file                         |
| `-log`     | `statsrv.log` | Log file, appended to                        |
| `-max`     | `100`         | Most matches a single GET can return         |
| `-maxbody` | `1048576`     | Largest POST body accepted, in bytes         |

Examples:

```sh
# all defaults: port 47910 on every interface, files in the current directory
./statsrv

# a different port
./statsrv -addr :8080

# only listen on one interface, eg a private network shared with game servers
./statsrv -addr 10.0.0.5:47910

# keep the database and log somewhere permanent
./statsrv -db /var/lib/statsrv/stats.db -log /var/log/statsrv/statsrv.log

# never return more than 25 matches per request
./statsrv -max 25

# accept POST bodies up to 4 MB
./statsrv -maxbody 4194304
```

statsrv only speaks plain HTTP. If you need HTTPS, put it behind a reverse
proxy such as nginx or Caddy.

### As a systemd service

`statsrv.service` runs statsrv at boot and restarts it if it exits. It runs as
an unprivileged dynamic user. The database goes in `/var/lib/statsrv` and the
log in `/var/log/statsrv`; systemd creates both directories.

```sh
make statsrv
sudo cp statsrv /usr/local/bin/
sudo cp statsrv.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now statsrv
```

To change the flags, edit the `ExecStart=` line, or run
`systemctl edit statsrv` to override it.

## Configuring the game server

In the OpenTDM server config:

```
set g_send_stats 1
set g_stats_url "http://stats.example.com:47910/stats"
```

## API

Both endpoints are at `/stats`.

### POST /stats

Stores one match. The game server sends this automatically when the match
ends. statsrv records when it received the match (`received`) and the address
it came from (`source`).

```sh
curl -X POST -H 'Content-Type: application/json' \
    --data-binary @match.json http://localhost:47910/stats
```

```json
{"id":42}
```

A body that isn't valid JSON, or has a bad `time` or player `team`, gets a
`400` with `{"error":"..."}`.

### GET /stats?since=DATE[&limit=N]

Returns the matches that ended at or after `since`, oldest first.

`since` accepts any of these forms. Times without a time zone are taken as UTC.

- `2026-10-01`
- `2026-10-01 18:30:00` (encode the space as `%20` in a URL)
- `2026-10-01T18:30:00`
- `2026-10-01T18:30:00Z` or `2026-10-01T14:30:00-04:00` (RFC 3339)

`limit` asks for fewer matches. It can't go above `-max`.

```sh
curl 'http://localhost:47910/stats?since=2026-10-01'
curl 'http://localhost:47910/stats?since=2026-10-01T18:30:00Z&limit=10'
```

```json
{
  "since": "2026-10-01T00:00:00Z",
  "count": 1,
  "more": false,
  "matches": [
    {
      "id": 42,
      "time": "2026-10-05T21:14:02Z",
      "received": "2026-10-05T21:14:02Z",
      "source": "203.0.113.7",
      "map": "q2dm1",
      "demo": "...",
      "demo_hostname": null,
      "home_team": "A",
      "home_score": 10,
      "away_team": "B",
      "away_score": 5,
      "players": [
        {
          "name": "player",
          "stats_id": "...",
          "team": "home",
          "kills": 10,
          "...": "...",
          "items": [
            {"name": "Railgun", "classname": "weapon_railgun", "accuracy": 55.5, "...": "..."}
          ]
        }
      ],
      "spectators": [{"name": "watcher", "stats_id": "..."}]
    }
  ]
}
```

If `more` is `true`, there were more matches than the limit allowed. To get
them, ask again with `since` set to the last match's `time`. `since` includes
matches at exactly that time, so skip any `id` you already have.

## Database

`schema.sql` defines four tables:

- `matches`: one row per match.
- `players`: each player in a match.
- `player_items`: each player's weapon and item stats.
- `spectators`: who was watching.

Times are stored in UTC as `YYYY-MM-DDTHH:MM:SSZ`, so they sort correctly as
text. You can query the database directly with `sqlite3`:

```sh
sqlite3 statsrv.db "SELECT name, SUM(kills) FROM players
                    GROUP BY stats_id ORDER BY 2 DESC LIMIT 10"
```

## Security

The POST endpoint has no authentication: anyone who can reach the port can
add matches. Firewall the port so only your game servers can reach it, or
listen on a private address with `-addr`.
