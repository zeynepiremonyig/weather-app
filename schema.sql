-- SCHEMA.SQL
-- This file is the blueprint of the SQLite database.
-- It defines which tables exist and what each column stores.

-- ==========================================
-- TABLE 1: Users
-- ==========================================
-- We create a table named 'Users' if it doesn't already exist.
CREATE TABLE IF NOT EXISTS Users (
    -- id: Unique number for each user.
    -- PRIMARY KEY + AUTOINCREMENT means SQLite creates new IDs automatically.
    id INTEGER PRIMARY KEY AUTOINCREMENT,

    -- username: Login name of the user.
    -- UNIQUE prevents duplicate usernames.
    -- NOT NULL prevents empty value at database level.
    username TEXT UNIQUE NOT NULL,

    -- password_hash: Hashed password value (never store plain text passwords).
    password_hash TEXT NOT NULL
);

-- ==========================================
-- TABLE 2: QueryLogs
-- ==========================================
CREATE TABLE IF NOT EXISTS QueryLogs (
    -- id: Unique ID of each search log entry.
    id INTEGER PRIMARY KEY AUTOINCREMENT,

    -- user_id: ID of the user who made the search.
    -- This value links to Users.id as a foreign key.
    user_id INTEGER NOT NULL,

    -- city: City name entered by user (for example, "Istanbul").
    city TEXT NOT NULL,

    -- timestamp: Search time.
    -- DEFAULT CURRENT_TIMESTAMP stores current time automatically.
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,

    -- result_summary: Short weather snapshot (for example, "15.0 C, Sunny").
    result_summary TEXT NOT NULL,

    -- Foreign key relationship between QueryLogs.user_id and Users.id.
    FOREIGN KEY (user_id) REFERENCES Users(id)
);