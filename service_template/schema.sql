-- Таблица пользователей
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    login VARCHAR(64) NOT NULL UNIQUE,
    password_hash VARCHAR(256) NOT NULL,
    first_name VARCHAR(128) NOT NULL,
    last_name VARCHAR(128) NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT NOW()
);

-- Таблица записей стены(owner_id  — чья стена, author_id — кто написал запись)
CREATE TABLE wall_posts (
    id SERIAL PRIMARY KEY,
    owner_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    author_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    content TEXT NOT NULL CHECK (content <> ''),
    created_at TIMESTAMP NOT NULL DEFAULT NOW()
);

-- Таблица сообщений чата
CREATE TABLE chat_messages (
    id SERIAL PRIMARY KEY,
    sender_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    receiver_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    text TEXT NOT NULL CHECK (text <> ''),
    created_at  TIMESTAMP NOT NULL DEFAULT NOW(),

    -- нельзя отправить сообщение самому себе
    CONSTRAINT no_self_message CHECK (sender_id <> receiver_id)
);

-- Поиск пользователя по логину
CREATE UNIQUE INDEX idx_users_login
    ON users (login);

-- Поиск пользователей по маске 
CREATE EXTENSION IF NOT EXISTS pg_trgm;

CREATE INDEX idx_users_fullname_trgm
    ON users USING GIN ((first_name || ' ' || last_name) gin_trgm_ops);

-- Загрузка стены пользователя: фильтр по owner_id + сортировка по дате
CREATE INDEX idx_wall_posts_owner_created
    ON wall_posts (owner_id, created_at DESC);

CREATE INDEX idx_wall_posts_author_id
    ON wall_posts (author_id);

-- Получение входящих сообщений пользователя
CREATE INDEX idx_chat_messages_receiver_id
    ON chat_messages (receiver_id);

-- Получение исходящих сообщений пользователя
CREATE INDEX idx_chat_messages_sender_id
    ON chat_messages (sender_id);
