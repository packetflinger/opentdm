GO      ?= go
SQLITE3 ?= sqlite3

BIN := statsrv
DB  ?= statsrv.db

SRCS := $(wildcard *.go) schema.sql go.mod go.sum

.PHONY: all db clean

all: $(BIN) db

$(BIN): $(SRCS)
	$(GO) build -o $@ .

# creates the database only if it doesn't exist yet, an existing one is left
# alone (statsrv also applies the schema itself when it starts)
db: $(DB)

$(DB):
	$(SQLITE3) $@ < schema.sql

clean:
	rm -f $(BIN)
