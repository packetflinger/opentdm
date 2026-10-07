package main

import (
	"database/sql"
	_ "embed"
	"fmt"
	"net/url"
	"strings"

	_ "modernc.org/sqlite"
)

//go:embed schema.sql
var schema string

// Match is the document an OpenTDM server POSTs when a match ends, see
// TDM_BuildMatchStatsJSON in g_tdm_stats.c. ID, Received and Source are
// filled in by statsrv and only appear in GET responses.
type Match struct {
	ID           int64       `json:"id"`
	Time         string      `json:"time"`
	Received     string      `json:"received"`
	Source       string      `json:"source"`
	Map          *string     `json:"map"`
	Demo         *string     `json:"demo"`
	DemoHostname *string     `json:"demo_hostname"`
	HomeTeam     *string     `json:"home_team"`
	HomeScore    int         `json:"home_score"`
	AwayTeam     *string     `json:"away_team"`
	AwayScore    int         `json:"away_score"`
	Players      []Player    `json:"players"`
	Spectators   []Spectator `json:"spectators"`
}

type Player struct {
	Name               *string `json:"name"`
	StatsID            *string `json:"stats_id"`
	Team               string  `json:"team"`
	Kills              int     `json:"kills"`
	Deaths             int     `json:"deaths"`
	Suicides           int     `json:"suicides"`
	TeamKills          int     `json:"team_kills"`
	Telefrags          int     `json:"telefrags"`
	DamageDealt        int     `json:"damage_dealt"`
	DamageReceived     int     `json:"damage_received"`
	TeamDamageDealt    int     `json:"team_damage_dealt"`
	TeamDamageReceived int     `json:"team_damage_received"`
	Items              []Item  `json:"items"`
}

type Item struct {
	Name           *string  `json:"name"`
	Classname      *string  `json:"classname"`
	Accuracy       *float64 `json:"accuracy"`
	Shots          int      `json:"shots"`
	Hits           int      `json:"hits"`
	Kills          int      `json:"kills"`
	Deaths         int      `json:"deaths"`
	DamageDealt    int      `json:"damage_dealt"`
	DamageReceived int      `json:"damage_received"`
	Pickups        int      `json:"pickups"`
	Missed         int      `json:"missed"`
}

type Spectator struct {
	Name    *string `json:"name"`
	StatsID *string `json:"stats_id"`
}

func openDB(path string) (*sql.DB, error) {
	dsn := "file:" + (&url.URL{Path: path}).EscapedPath() +
		"?_pragma=foreign_keys(1)&_pragma=journal_mode(WAL)&_pragma=busy_timeout(5000)"
	db, err := sql.Open("sqlite", dsn)
	if err != nil {
		return nil, err
	}
	// one connection serializes writers, plenty for a few posts per match
	db.SetMaxOpenConns(1)
	if _, err := db.Exec(schema); err != nil {
		db.Close()
		return nil, fmt.Errorf("creating schema: %w", err)
	}
	return db, nil
}

// insertMatch stores m and its players, items and spectators, returning the
// new match id.
func insertMatch(db *sql.DB, m *Match) (int64, error) {
	tx, err := db.Begin()
	if err != nil {
		return 0, err
	}
	defer tx.Rollback()

	res, err := tx.Exec(`INSERT INTO matches (time, received, source, map, demo,
		demo_hostname, home_team, home_score, away_team, away_score)
		VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`,
		m.Time, m.Received, m.Source, m.Map, m.Demo, m.DemoHostname,
		m.HomeTeam, m.HomeScore, m.AwayTeam, m.AwayScore)
	if err != nil {
		return 0, err
	}
	matchID, err := res.LastInsertId()
	if err != nil {
		return 0, err
	}

	for _, p := range m.Players {
		res, err := tx.Exec(`INSERT INTO players (match_id, name, stats_id, team,
			kills, deaths, suicides, team_kills, telefrags, damage_dealt,
			damage_received, team_damage_dealt, team_damage_received)
			VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`,
			matchID, p.Name, p.StatsID, p.Team, p.Kills, p.Deaths, p.Suicides,
			p.TeamKills, p.Telefrags, p.DamageDealt, p.DamageReceived,
			p.TeamDamageDealt, p.TeamDamageReceived)
		if err != nil {
			return 0, err
		}
		playerID, err := res.LastInsertId()
		if err != nil {
			return 0, err
		}

		for _, it := range p.Items {
			_, err := tx.Exec(`INSERT INTO player_items (player_id, name,
				classname, accuracy, shots, hits, kills, deaths, damage_dealt,
				damage_received, pickups, missed)
				VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`,
				playerID, it.Name, it.Classname, it.Accuracy, it.Shots, it.Hits,
				it.Kills, it.Deaths, it.DamageDealt, it.DamageReceived,
				it.Pickups, it.Missed)
			if err != nil {
				return 0, err
			}
		}
	}

	for _, s := range m.Spectators {
		_, err := tx.Exec(`INSERT INTO spectators (match_id, name, stats_id)
			VALUES (?, ?, ?)`, matchID, s.Name, s.StatsID)
		if err != nil {
			return 0, err
		}
	}

	return matchID, tx.Commit()
}

// matchesSince returns up to limit matches that ended at or after since
// (normalized UTC time), oldest first.
func matchesSince(db *sql.DB, since string, limit int) ([]*Match, error) {
	rows, err := db.Query(`SELECT id, time, received, source, map, demo,
		demo_hostname, home_team, home_score, away_team, away_score
		FROM matches WHERE time >= ? ORDER BY time, id LIMIT ?`, since, limit)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	matches := []*Match{}
	byID := map[int64]*Match{}
	for rows.Next() {
		m := &Match{Players: []Player{}, Spectators: []Spectator{}}
		err := rows.Scan(&m.ID, &m.Time, &m.Received, &m.Source, &m.Map,
			&m.Demo, &m.DemoHostname, &m.HomeTeam, &m.HomeScore, &m.AwayTeam,
			&m.AwayScore)
		if err != nil {
			return nil, err
		}
		matches = append(matches, m)
		byID[m.ID] = m
	}
	if err := rows.Err(); err != nil {
		return nil, err
	}
	if len(matches) == 0 {
		return matches, nil
	}

	ids := make([]any, len(matches))
	for i, m := range matches {
		ids[i] = m.ID
	}
	in := "(" + strings.TrimSuffix(strings.Repeat("?,", len(ids)), ",") + ")"

	// players and their items, in the order they were posted
	rows, err = db.Query(`SELECT p.id, p.match_id, p.name, p.stats_id, p.team,
		p.kills, p.deaths, p.suicides, p.team_kills, p.telefrags,
		p.damage_dealt, p.damage_received, p.team_damage_dealt,
		p.team_damage_received
		FROM players p WHERE p.match_id IN `+in+` ORDER BY p.id`, ids...)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	type playerRef struct {
		match *Match
		index int
	}
	players := map[int64]playerRef{}
	for rows.Next() {
		var playerID, matchID int64
		p := Player{Items: []Item{}}
		err := rows.Scan(&playerID, &matchID, &p.Name, &p.StatsID, &p.Team,
			&p.Kills, &p.Deaths, &p.Suicides, &p.TeamKills, &p.Telefrags,
			&p.DamageDealt, &p.DamageReceived, &p.TeamDamageDealt,
			&p.TeamDamageReceived)
		if err != nil {
			return nil, err
		}
		m := byID[matchID]
		m.Players = append(m.Players, p)
		players[playerID] = playerRef{m, len(m.Players) - 1}
	}
	if err := rows.Err(); err != nil {
		return nil, err
	}

	rows, err = db.Query(`SELECT i.player_id, i.name, i.classname, i.accuracy,
		i.shots, i.hits, i.kills, i.deaths, i.damage_dealt, i.damage_received,
		i.pickups, i.missed
		FROM player_items i JOIN players p ON p.id = i.player_id
		WHERE p.match_id IN `+in+` ORDER BY i.id`, ids...)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	for rows.Next() {
		var playerID int64
		var it Item
		err := rows.Scan(&playerID, &it.Name, &it.Classname, &it.Accuracy,
			&it.Shots, &it.Hits, &it.Kills, &it.Deaths, &it.DamageDealt,
			&it.DamageReceived, &it.Pickups, &it.Missed)
		if err != nil {
			return nil, err
		}
		ref := players[playerID]
		p := &ref.match.Players[ref.index]
		p.Items = append(p.Items, it)
	}
	if err := rows.Err(); err != nil {
		return nil, err
	}

	rows, err = db.Query(`SELECT match_id, name, stats_id FROM spectators
		WHERE match_id IN `+in+` ORDER BY id`, ids...)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	for rows.Next() {
		var matchID int64
		var s Spectator
		if err := rows.Scan(&matchID, &s.Name, &s.StatsID); err != nil {
			return nil, err
		}
		m := byID[matchID]
		m.Spectators = append(m.Spectators, s)
	}
	return matches, rows.Err()
}
