## 1. Поиск пользователя по логину

**Запрос:**
```sql
SELECT id, login, first_name, last_name
FROM users
WHERE login = 'ivan_ivanov';
```

**EXPLAIN без индекса:**
```
 Seq Scan on users  (cost=0.00..10.75 rows=1 width=698) (actual time=0.027..0.029 rows=1 loops=1)
   Filter: ((login)::text = 'ivan_ivanov'::text)
   Rows Removed by Filter: 11
   Buffers: shared hit=1
 Planning Time: 0.292 ms
 Execution Time: 0.119 ms
```
PostgreSQL перебирает все строки таблицы — O(N).

**Оптимизация — уникальный B-tree индекс:**
```sql
CREATE UNIQUE INDEX idx_users_login ON users (login);
```

**EXPLAIN после оптимизации:**
```
 Index Scan using idx_users_login on users  (cost=0.14..8.16 rows=1 width=698) (actual time=0.079..0.081 rows=1 loops=1)
   Index Cond: ((login)::text = 'ivan_ivanov'::text)
   Buffers: shared hit=2
 Planning Time: 0.275 ms
 Execution Time: 0.138 ms
```
**Вывод:** до оптимизации PostgreSQL делал Seq Scan — перебирал все строки таблицы подряд (cost=0.00..10.75). После добавления индекса план изменился на Index Scan — база сразу переходит к нужной строке по дереву (cost=0.14..8.16). Время выполнения на тестовых данных почти одинаковое (0.119 мс vs 0.138 мс), потому что строк всего 12. На реальной таблице с тысячами пользователей индекс даст значительное ускорение.


## 2. Поиск пользователя по маске имени/фамилии

**Запрос:**
```sql
SELECT id, login, first_name, last_name
FROM users
WHERE (first_name || ' ' || last_name) ILIKE '%иванов%';
```

**EXPLAIN без индекса:**
```
 Seq Scan on users  (cost=0.00..11.05 rows=1 width=698) (actual time=0.201..0.232 rows=1 loops=1)
   Filter: ((((first_name)::text || ' '::text) || (last_name)::text) ~~* '%иванов%'::text)
   Rows Removed by Filter: 11
   Buffers: shared hit=1
 Planning:
   Buffers: shared hit=9 read=3 dirtied=1
 Planning Time: 48.079 ms
 Execution Time: 0.284 ms
```
Паттерн вида `%слово%` — обычные B-tree индексы не помогают, так как нужно искать подстроку в середине строки.

**Оптимизация — GIN индекс с триграммами (pg_trgm):**
```sql
CREATE EXTENSION IF NOT EXISTS pg_trgm;

CREATE INDEX idx_users_fullname_trgm
    ON users USING GIN ((first_name || ' ' || last_name) gin_trgm_ops);
```
Триграммы — разбиение строки на тройки символов ("ива", "ван", "ано", "нов", ...).
GIN-индекс хранит все триграммы и быстро находит строки, содержащие нужные.

**EXPLAIN после оптимизации:**
```
Seq Scan on users  (cost=0.00..11.05 rows=1 width=698) (actual time=0.042..0.070 rows=1 loops=1)
   Filter: ((((first_name)::text || ' '::text) || (last_name)::text) ~~* '%иванов%'::text)
   Rows Removed by Filter: 11
   Buffers: shared hit=1
 Planning:
   Buffers: shared hit=1
 Planning Time: 0.800 ms
 Execution Time: 0.116 ms
```
**Вывод:** тип плана не изменился — оба раза Seq Scan с одинаковой стоимостью (0.00..11.05). На 12 строках PostgreSQL не видит смысла использовать индекс. Но видна другая разница: время планирования упало с 48 мс до 0.8 мс (при первом запуске PostgreSQL загружал расширение pg_trgm), время выполнения сократилось с 0.284 мс до 0.116 мс. На таблице с тысячами записей GIN-индекс начнёт применяться и поиск по маске имени значительно ускорится.


## 3. Загрузка стены пользователя

**Запрос:**
```sql
SELECT wp.id, wp.content, wp.created_at, u.login
FROM wall_posts wp
JOIN users u ON u.id = wp.author_id
WHERE wp.owner_id = 1
ORDER BY wp.created_at DESC;
```

**EXPLAIN без индексов:**
```
 Sort  (cost=33.72..33.73 rows=5 width=190) (actual time=2.600..2.604 rows=3 loops=1)
   Sort Key: wp.created_at DESC
   Sort Method: quicksort  Memory: 25kB
   Buffers: shared hit=4 read=1 dirtied=1
   ->  Hash Join  (cost=22.81..33.66 rows=5 width=190) (actual time=0.883..0.892 rows=3 loops=1)
         Hash Cond: (u.id = wp.author_id)
         Buffers: shared hit=1 read=1 dirtied=1
         ->  Seq Scan on users u  (cost=0.00..10.60 rows=60 width=150) (actual time=0.139..0.141 rows=12 loops=1)
               Buffers: shared hit=1
         ->  Hash  (cost=22.75..22.75 rows=5 width=48) (actual time=0.631..0.632 rows=3 loops=1)
               Buckets: 1024  Batches: 1  Memory Usage: 9kB
               Buffers: shared read=1 dirtied=1
               ->  Seq Scan on wall_posts wp  (cost=0.00..22.75 rows=5 width=48) (actual time=0.536..0.540 rows=3 loops=1)
                     Filter: (owner_id = 1)
                     Rows Removed by Filter: 9
                     Buffers: shared read=1 dirtied=1
 Planning:
   Buffers: shared hit=148 read=4 dirtied=1
 Planning Time: 13.265 ms
 Execution Time: 3.700 ms
```
Два Seq Scan на таблицах + отдельный шаг Sort для сортировки результатов.

**Оптимизация:**
```sql
-- Составной индекс: фильтрует по owner_id и заодно сортирует по created_at
CREATE INDEX idx_wall_posts_owner_created
    ON wall_posts (owner_id, created_at DESC);

-- Индекс для JOIN по author_id
CREATE INDEX idx_wall_posts_author_id
    ON wall_posts (author_id);
```

**EXPLAIN после оптимизации:**
```
Sort  (cost=23.63..23.64 rows=5 width=190) (actual time=6.153..6.157 rows=3 loops=1)
   Sort Key: wp.created_at DESC
   Sort Method: quicksort  Memory: 25kB
   Buffers: shared hit=2 read=1
   ->  Hash Join  (cost=12.72..23.57 rows=5 width=190) (actual time=6.097..6.109 rows=3 loops=1)
         Hash Cond: (u.id = wp.author_id)
         Buffers: shared hit=2 read=1
         ->  Seq Scan on users u  (cost=0.00..10.60 rows=60 width=150) (actual time=0.024..0.028 rows=12 loops=1)
               Buffers: shared hit=1
         ->  Hash  (cost=12.66..12.66 rows=5 width=48) (actual time=5.974..5.975 rows=3 loops=1)
               Buckets: 1024  Batches: 1  Memory Usage: 9kB
               Buffers: shared hit=1 read=1
               ->  Bitmap Heap Scan on wall_posts wp  (cost=4.19..12.66 rows=5 width=48) (actual time=5.917..5.920 rows=3 loops=1)
                     Recheck Cond: (owner_id = 1)
                     Heap Blocks: exact=1
                     Buffers: shared hit=1 read=1
                     ->  Bitmap Index Scan on idx_wall_posts_owner_created  (cost=0.00..4.19 rows=5 width=0) (actual time=5.827..5.828 rows=3 loops=1)    
                           Index Cond: (owner_id = 1)
                           Buffers: shared read=1
 Planning Time: 0.376 ms
 Execution Time: 46.604 ms
```
**Вывод:** до оптимизации PostgreSQL делал Seq Scan по всей таблице `wall_posts`, отфильтровывая 9 из 12 строк. После — появился Bitmap Index Scan: база сначала строит битовую карту по индексу, затем читает только нужные строки. Расчётная стоимость снизилась с 33.72 до 23.63, время планирования упало с 13 мс до 0.4 мс. Фактическое время выполнения выросло (3.7 мс → 46.6 мс) — страницы индекса читались с диска в первый раз. При повторных запросах они окажутся в кэше и время снизится.


## 4. Получение сообщений для пользователя

**Запрос:**
```sql
SELECT id, sender_id, receiver_id, text, created_at
FROM chat_messages
WHERE sender_id = 1 OR receiver_id = 1
ORDER BY created_at DESC;
```

**EXPLAIN без индексов:**
```
Sort  (cost=25.47..25.49 rows=10 width=52) (actual time=3.666..3.668 rows=4 loops=1)
   Sort Key: created_at DESC
   Sort Method: quicksort  Memory: 25kB
   Buffers: shared read=1 dirtied=1
   ->  Seq Scan on chat_messages  (cost=0.00..25.30 rows=10 width=52) (actual time=3.631..3.636 rows=4 loops=1)
         Filter: ((sender_id = 1) OR (receiver_id = 1))
         Rows Removed by Filter: 8
         Buffers: shared read=1 dirtied=1
 Planning:
   Buffers: shared hit=48 read=3
 Planning Time: 4.463 ms
 Execution Time: 3.742 ms
```

**Оптимизация:**
```sql
CREATE INDEX idx_chat_messages_sender_id   ON chat_messages (sender_id);
CREATE INDEX idx_chat_messages_receiver_id ON chat_messages (receiver_id);
```
PostgreSQL объединит результаты двух индексов через BitmapOr.

**EXPLAIN после оптимизации:**
```
Sort  (cost=19.13..19.15 rows=10 width=52) (actual time=41.895..41.899 rows=4 loops=1)
   Sort Key: created_at DESC
   Sort Method: quicksort  Memory: 25kB
   Buffers: shared hit=1 read=2
   ->  Bitmap Heap Scan on chat_messages  (cost=8.38..18.96 rows=10 width=52) (actual time=41.845..41.850 rows=4 loops=1)
         Recheck Cond: ((sender_id = 1) OR (receiver_id = 1))
         Heap Blocks: exact=1
         Buffers: shared hit=1 read=2
         ->  BitmapOr  (cost=8.38..8.38 rows=10 width=0) (actual time=41.823..41.824 rows=0 loops=1)
               Buffers: shared read=2
               ->  Bitmap Index Scan on idx_chat_messages_sender_id  (cost=0.00..4.19 rows=5 width=0) (actual time=40.776..40.777 rows=2 loops=1)                     Index Cond: (sender_id = 1)
                     Buffers: shared read=1
               ->  Bitmap Index Scan on idx_chat_messages_receiver_id  (cost=0.00..4.19 rows=5 width=0) (actual time=1.041..1.041 rows=2 loops=1)                     Index Cond: (receiver_id = 1)
                     Buffers: shared read=1
 Planning Time: 0.188 ms
 Execution Time: 42.215 ms
```
**Вывод:** до оптимизации был Seq Scan — база перебирала все сообщения и отбрасывала ненужные (Rows Removed by Filter: 8). После — появился Bitmap Heap Scan: PostgreSQL использует оба индекса одновременно, объединяет результаты через BitmapOr и читает только нужные строки. Расчётная стоимость снизилась с 25.47 до 19.13, время планирования с 4.5 мс до 0.2 мс. Фактическое время выросло (3.7 мс → 42 мс) из-за чтения страниц индекса с диска — это разовый эффект при первом запуске.


## Сводная таблица индексов

| Индекс | Таблица | Колонки | Тип | Зачем нужен |
|--------|---------|---------|-----|-------------|
| `idx_users_login` | users | login | B-tree UNIQUE | Поиск по логину, уникальность |
| `idx_users_fullname_trgm` | users | first_name \|\| last_name | GIN (pg_trgm) | Поиск по маске ILIKE '%...%' |
| `idx_wall_posts_owner_created` | wall_posts | owner_id, created_at DESC | B-tree | Загрузка стены с сортировкой |
| `idx_wall_posts_author_id` | wall_posts | author_id | B-tree | FK, JOIN при загрузке стены |
| `idx_chat_messages_receiver_id` | chat_messages | receiver_id | B-tree | Входящие сообщения |
| `idx_chat_messages_sender_id` | chat_messages | sender_id | B-tree | Исходящие сообщения |

---

## 5. Партиционирование

### Таблица `chat_messages`

Сообщения накапливаются быстрее всего. Рекомендуется **RANGE-партиционирование по `created_at`** — по кварталам:

```sql
-- Создаём таблицу с партиционированием
CREATE TABLE chat_messages (
    id          SERIAL    NOT NULL,
    sender_id   INT       NOT NULL REFERENCES users(id),
    receiver_id INT       NOT NULL REFERENCES users(id),
    text        TEXT      NOT NULL,
    created_at  TIMESTAMP NOT NULL DEFAULT NOW(),
    CONSTRAINT no_self_message CHECK (sender_id <> receiver_id)
) PARTITION BY RANGE (created_at);

-- Партиция за 1-й квартал 2024
CREATE TABLE chat_messages_2024_q1
    PARTITION OF chat_messages
    FOR VALUES FROM ('2024-01-01') TO ('2024-04-01');

-- Партиция за 2-й квартал 2024
CREATE TABLE chat_messages_2024_q2
    PARTITION OF chat_messages
    FOR VALUES FROM ('2024-04-01') TO ('2024-07-01');
```
