// statsrv collects the match stats OpenTDM servers POST at the end of each
// match (g_send_stats 1, g_stats_url http://host:47910/stats), stores them in
// SQLite and serves them back as JSON:
//
//	GET /stats?since=2026-10-01[&limit=N]
package main

import (
	"database/sql"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"log"
	"net"
	"net/http"
	"os"
	"strconv"
	"time"
)

const timeFormat = "2006-01-02T15:04:05Z"

// accepted forms of the "since" parameter, zoneless ones are taken as UTC
var sinceFormats = []string{
	time.RFC3339,
	"2006-01-02T15:04:05",
	"2006-01-02 15:04:05",
	"2006-01-02",
}

type server struct {
	db       *sql.DB
	maxLimit int
	maxBody  int64
}

func main() {
	addr := flag.String("addr", ":47910", "address and port to listen on")
	dbPath := flag.String("db", "statsrv.db", "SQLite database file")
	logPath := flag.String("log", "statsrv.log", "log file")
	maxLimit := flag.Int("max", 100, "most matches returned by one GET")
	maxBody := flag.Int64("maxbody", 1<<20, "largest POST body accepted, in bytes")
	flag.Parse()

	logFile, err := os.OpenFile(*logPath, os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0644)
	if err != nil {
		log.Fatalf("opening log file: %v", err)
	}
	log.SetOutput(logFile)

	if *maxLimit < 1 {
		log.Fatalf("-max must be at least 1")
	}

	db, err := openDB(*dbPath)
	if err != nil {
		log.Fatalf("opening database %s: %v", *dbPath, err)
	}
	defer db.Close()

	s := &server{db: db, maxLimit: *maxLimit, maxBody: *maxBody}
	mux := http.NewServeMux()
	mux.HandleFunc("POST /stats", s.postStats)
	mux.HandleFunc("GET /stats", s.getStats)

	srv := &http.Server{
		Addr:              *addr,
		Handler:           mux,
		ReadHeaderTimeout: 10 * time.Second,
		ReadTimeout:       30 * time.Second,
		WriteTimeout:      60 * time.Second,
		IdleTimeout:       120 * time.Second,
	}
	log.Printf("listening on %s, database %s", *addr, *dbPath)
	log.Fatal(srv.ListenAndServe())
}

// postStats stores one match document from a game server.
func (s *server) postStats(w http.ResponseWriter, r *http.Request) {
	source, _, err := net.SplitHostPort(r.RemoteAddr)
	if err != nil {
		source = r.RemoteAddr
	}

	var m Match
	dec := json.NewDecoder(http.MaxBytesReader(w, r.Body, s.maxBody))
	if err := dec.Decode(&m); err != nil {
		s.fail(w, r, http.StatusBadRequest, fmt.Sprintf("bad JSON: %v", err))
		return
	}

	t, err := time.Parse(time.RFC3339, m.Time)
	if err != nil {
		s.fail(w, r, http.StatusBadRequest, fmt.Sprintf("bad time %q", m.Time))
		return
	}
	for _, p := range m.Players {
		if p.Team != "home" && p.Team != "away" {
			s.fail(w, r, http.StatusBadRequest, fmt.Sprintf("bad team %q", p.Team))
			return
		}
	}

	m.Time = t.UTC().Format(timeFormat)
	m.Received = time.Now().UTC().Format(timeFormat)
	m.Source = source

	id, err := insertMatch(s.db, &m)
	if err != nil {
		s.fail(w, r, http.StatusInternalServerError, fmt.Sprintf("storing match: %v", err))
		return
	}

	log.Printf("%s: stored match %d (%s on %s, %d players)", source, id, m.Time,
		deref(m.Map), len(m.Players))
	writeJSON(w, http.StatusCreated, map[string]int64{"id": id})
}

// getStats returns matches that ended at or after the "since" parameter,
// oldest first. "more" tells the caller to ask again from the last match's
// time to get the rest; since is inclusive, so skip the ids already seen.
func (s *server) getStats(w http.ResponseWriter, r *http.Request) {
	q := r.URL.Query()

	sinceParam := q.Get("since")
	if sinceParam == "" {
		s.fail(w, r, http.StatusBadRequest, "missing since parameter")
		return
	}
	since, err := parseSince(sinceParam)
	if err != nil {
		s.fail(w, r, http.StatusBadRequest, err.Error())
		return
	}

	limit := s.maxLimit
	if l := q.Get("limit"); l != "" {
		n, err := strconv.Atoi(l)
		if err != nil || n < 1 {
			s.fail(w, r, http.StatusBadRequest, fmt.Sprintf("bad limit %q", l))
			return
		}
		limit = min(n, s.maxLimit)
	}

	// fetch one extra to know whether there are more
	matches, err := matchesSince(s.db, since.Format(timeFormat), limit+1)
	if err != nil {
		s.fail(w, r, http.StatusInternalServerError, fmt.Sprintf("reading matches: %v", err))
		return
	}
	more := len(matches) > limit
	if more {
		matches = matches[:limit]
	}

	log.Printf("%s: GET since %s, returned %d matches", r.RemoteAddr,
		since.Format(timeFormat), len(matches))
	writeJSON(w, http.StatusOK, map[string]any{
		"since":   since.Format(timeFormat),
		"count":   len(matches),
		"more":    more,
		"matches": matches,
	})
}

func parseSince(v string) (time.Time, error) {
	for _, f := range sinceFormats {
		if t, err := time.Parse(f, v); err == nil {
			return t.UTC(), nil
		}
	}
	return time.Time{}, errors.New("bad since parameter, use YYYY-MM-DD, " +
		"YYYY-MM-DD HH:MM:SS (UTC) or RFC 3339")
}

func (s *server) fail(w http.ResponseWriter, r *http.Request, code int, msg string) {
	log.Printf("%s: %s %s: %d %s", r.RemoteAddr, r.Method, r.URL, code, msg)
	writeJSON(w, code, map[string]string{"error": msg})
}

func writeJSON(w http.ResponseWriter, code int, v any) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(code)
	if err := json.NewEncoder(w).Encode(v); err != nil {
		log.Printf("writing response: %v", err)
	}
}

func deref(s *string) string {
	if s == nil {
		return ""
	}
	return *s
}
