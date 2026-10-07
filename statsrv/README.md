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
./statsrv -post-token <token> [flags]
```

| Flag       | Default       | Description                                  |
|------------|---------------|----------------------------------------------|
| `-addr`    | `:47910`      | Address and port to listen on                |
| `-db`      | `statsrv.db`  | SQLite database file                         |
| `-log`     | `statsrv.log` | Log file, appended to                        |
| `-max`     | `100`         | Most matches a single GET can return         |
| `-maxbody` | `1048576`     | Largest POST body accepted, in bytes         |
| `-post-token` | *(none)*   | Token game servers must send to POST stats; must match their `g_stats_token`. **Required**, statsrv won't start without it |
| `-get-token`  | *(none)*   | Token needed to GET stats, passed as `?token=`. Blank lets anyone read them |

Examples:

```sh
# defaults: port 47910 on every interface, files in the current directory,
# anyone can read the stats
./statsrv -post-token s3cret

# also require a token to read the stats
./statsrv -post-token s3cret -get-token readme

# a different port
./statsrv -post-token s3cret -addr :8080

# only listen on one interface, eg a private network shared with game servers
./statsrv -post-token s3cret -addr 10.0.0.5:47910

# keep the database and log somewhere permanent
./statsrv -post-token s3cret -db /var/lib/statsrv/stats.db \
    -log /var/log/statsrv/statsrv.log

# never return more than 25 matches per request
./statsrv -post-token s3cret -max 25

# accept POST bodies up to 4 MB
./statsrv -post-token s3cret -maxbody 4194304
```

## Tokens

A missing or wrong token gets a `401`.

- **POST** needs the `-post-token` value in an `Authorization: Bearer <token>`
  header. Every game server sends its `g_stats_token` cvar this way, so set
  `-post-token` to the same value you give the game servers.
- **GET** needs the `-get-token` value, if one is set, in the `token` query
  parameter: `/stats?since=2026-10-01&token=readme`. It's one token shared by
  everyone you want to let read the stats, such as a website or a Discord
  bot. It controls who may read the stats, not who someone is.

Because the GET token is part of the URL, it can end up in browser history
and in the logs of any proxy in front of statsrv. statsrv blanks it out in
its own log.

statsrv only speaks plain HTTP, so tokens cross the network unencrypted.

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

Before starting it, change `-post-token` on the `ExecStart=` line to match
your game servers' `g_stats_token`, and add `-get-token` if you want reads to
need a token. To change the flags later, edit the `ExecStart=` line, or run
`systemctl edit statsrv` to override it.

## Configuring the game server

In the OpenTDM server config:

```
set g_send_stats 1
set g_stats_url "http://stats.example.com:47910/stats"
set g_stats_token "s3cret"
```

`g_stats_token` defaults to `changeme`. It must match statsrv's
`-post-token`, or statsrv rejects the stats.

## API

Both endpoints are at `/stats`.

### POST /stats

Stores one match. The game server sends this automatically when the match
ends. statsrv records when it received the match (`received`) and the address
it came from (`source`).

```sh
curl -X POST -H 'Content-Type: application/json' \
    -H 'Authorization: Bearer s3cret' \
    --data-binary @match.json http://localhost:47910/stats
```

```json
{"id":42}
```

A body that isn't valid JSON, or has a bad `time` or player `team`, gets a
`400` with `{"error":"..."}`.

### GET /stats?since=DATE[&limit=N][&token=TOKEN]

Returns the matches that ended at or after `since`, oldest first.

`since` accepts any of these forms. Times without a time zone are taken as UTC.

- `2026-10-01`
- `2026-10-01 18:30:00` (encode the space as `%20` in a URL)
- `2026-10-01T18:30:00`
- `2026-10-01T18:30:00Z` or `2026-10-01T14:30:00-04:00` (RFC 3339)

`limit` asks for fewer matches. It can't go above `-max`.

`token` is required when statsrv runs with `-get-token`, and must match it.

```sh
curl 'http://localhost:47910/stats?since=2026-10-01'
curl 'http://localhost:47910/stats?since=2026-10-01T18:30:00Z&limit=10'

# when statsrv runs with -get-token readme
curl 'http://localhost:47910/stats?since=2026-10-01&token=readme'
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

Only clients that send the `-post-token` value can add matches. Change it
from the game's default of `changeme`, because anyone who knows the token can
add made-up matches.

Without `-get-token`, anyone who can reach the port can read every match,
including each player's `stats_id`.

Tokens are sent over plain HTTP, so anyone who can watch the traffic between
a client and statsrv can read them. For more protection, firewall the port so
only your game servers and readers can reach it, listen on a private address
with `-addr`, or put statsrv behind an HTTPS reverse proxy.
