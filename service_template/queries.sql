-- Создание нового пользователя
INSERT INTO users (login, password_hash, first_name, last_name)
VALUES ('new_user', 'hash_newpass', 'Новый', 'Пользователь')
RETURNING id, login, first_name, last_name, created_at;

-- Поиск пользователя по логину
SELECT id, login, first_name, last_name
FROM users
WHERE login = 'ivan_ivanov';

-- Поиск пользователя по маске 
SELECT id, login, first_name, last_name
FROM users
WHERE (first_name || ' ' || last_name) ILIKE '%иванов%'
ORDER BY last_name, first_name;

-- Добавление записи на стену
INSERT INTO wall_posts (owner_id, author_id, content)
VALUES (1, 2, 'Привет, зашёл на твою стену!')
RETURNING id, owner_id, author_id, content, created_at;

-- Загрузка стены пользователя
SELECT
    wp.id,
    wp.content,
    wp.created_at,
    u.login      AS author_login,
    u.first_name AS author_first_name,
    u.last_name  AS author_last_name
FROM wall_posts wp
JOIN users u ON u.id = wp.author_id
WHERE wp.owner_id = 1
ORDER BY wp.created_at DESC;

-- Отправка сообщения пользователю
INSERT INTO chat_messages (sender_id, receiver_id, text)
VALUES (1, 2, 'Привет! Как дела?')
RETURNING id, sender_id, receiver_id, text, created_at;

-- Получение списка сообщений для пользователя
SELECT
    cm.id,
    cm.sender_id,
    cm.receiver_id,
    cm.text,
    cm.created_at,
    s.login AS sender_login,
    r.login AS receiver_login
FROM chat_messages cm
JOIN users s ON s.id = cm.sender_id
JOIN users r ON r.id = cm.receiver_id
WHERE cm.sender_id = 1 OR cm.receiver_id = 1
ORDER BY cm.created_at DESC;
