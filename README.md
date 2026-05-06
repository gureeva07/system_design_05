# ДЗ №5 - Оптимизация производительности через кеширование и rate limiting
## Вариант 1: Социальная сеть Facebook

**Автор:** Гуреева Алина, М8О-102СВ-25

---

## Что добавила

В прошлом задании сделала базовый API социальной сети. Здесь решала проблему производительности — поиск по логину и загрузка стены ходили в MongoDB на каждый запрос, даже если данные не менялись.

Я добавила:
- **Кеш** для двух эндпоинтов - поиск пользователя по логину и загрузка стены. Реализовала через простой in-memory словарь с TTL. Стратегия Cache-Aside: сначала смотрим в кеш, если нет, то идём в MongoDB, кладём результат в кеш.
- **Инвалидацию кеша** стены - когда добавляется новый пост, кеш сбрасывается сразу, иначе новый пост не был бы виден целую минуту.
- **Rate limiting** для поиска по имени - это самый дорогой запрос (regex по MongoDB), поэтому ограничила до 30 запросов в минуту на пользователя. Алгоритм Fixed Window Counter. При превышении возвращается HTTP 429.

---

## Архитектура хранилища

Использую две базы данных. PostgreSQL хранит логины и хэши паролей — это данные для авторизации, они строго структурированы и нужна надёжность. MongoDB хранит всё остальное: профили для поиска, посты на стене, сообщения. Выбрала MongoDB потому что там гибкая схема и удобно делать поиск по тексту (regex по имени и фамилии).

| База данных | Что хранит |
|-------------|-----------|
| PostgreSQL  | Пользователи (логин, хэш пароля) - авторизация и сессии |
| MongoDB     | Профили пользователей для поиска, посты на стене, сообщения чата |

Коллекции MongoDB:

| Коллекция      | Описание                              | Кешируется |
|----------------|---------------------------------------|-------------|
| `users`        | Профили пользователей для поиска      | Да, 5 минут |
| `wall_posts`   | Записи на стене                       | Да, 1 минута |
| `chat_messages`| Личные сообщения между пользователями | Нет |

Сообщения не кешировала — человек ждёт новых сообщений прямо сейчас, даже минута задержки была бы заметна.

---

## Запуск

```bash
cd service_template
docker-compose up --build
```

Остановить:
```bash
docker-compose down
```

---

## Проверка работы - реальные тесты

Все команды выполнялись в Git Bash.

### Шаг 1 - Регистрация и вход

```bash
curl -s -X POST http://localhost:8080/api/v1/auth/register \
  -H "Content-Type: application/json" \
  -d '{"login":"ivan2","password":"pass123","first_name":"Ivan","last_name":"Ivanov"}'
```

Ответ:
```json
{"id":16,"login":"ivan2"}
```

```bash
curl -s -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"login":"ivan2","password":"pass123"}'
```

Ответ:
```json
{"token":"4053890b3016014db3994469adaa688a","user_id":16}
```

---

### Шаг 2 - Проверка кеша (поиск пользователя по логину)

Первый запрос идёт в MongoDB - cache miss:

```bash
curl -s -w "\nВремя: %{time_total}s\n" \
  "http://localhost:8080/api/v1/users?login=ivan2" \
  -H "Authorization: Bearer 4053890b3016014db3994469adaa688a"
```

Ответ:
```json
{"login":"ivan2","first_name":"Ivan","last_name":"Ivanov"}
Время: 0.034794s
```

Второй запрос - cache hit, данные из памяти:

```bash
curl -s -w "\nВремя: %{time_total}s\n" \
  "http://localhost:8080/api/v1/users?login=ivan2" \
  -H "Authorization: Bearer 4053890b3016014db3994469adaa688a"
```

Ответ:
```json
{"login":"ivan2","first_name":"Ivan","last_name":"Ivanov"}
Время: 0.014436s
```

**Вывод:** второй запрос быстрее в 2.4 раза (34ms → 14ms) за счёт кеша. При большой нагрузке разница была бы ещё заметнее, потому что MongoDB здесь запущена локально,в реальном окружении сетевая задержка до БД добавляет десятки миллисекунд.

---

### Шаг 3 - Проверка инвалидации кеша стены

Смотрим стену - пусто, результат закешировался:

```bash
curl -s "http://localhost:8080/api/v1/wall/16" \
  -H "Authorization: Bearer 4053890b3016014db3994469adaa688a"
```

Ответ:
```json
[]
```

Добавляем пост - при этом кеш стены сбрасывается:

```bash
curl -s -X POST "http://localhost:8080/api/v1/wall/16" \
  -H "Authorization: Bearer 4053890b3016014db3994469adaa688a" \
  -H "Content-Type: application/json" \
  -d "{\"content\":\"hello world\"}"
```

Ответ:
```json
{"owner_id":16,"author_id":16,"content":"hello world","created_at":"2026-05-04 12:59:18"}
```

Смотрим стену снова - новый пост виден сразу:

```bash
curl -s "http://localhost:8080/api/v1/wall/16" \
  -H "Authorization: Bearer 4053890b3016014db3994469adaa688a"
```

Ответ:
```json
[{"owner_id":16,"author_id":16,"content":"hello world","created_at":"2026-05-04 12:59:18"}]
```

**Вывод:** без инвалидации кеша новый пост не был бы виден ещё 60 секунд (TTL стены = 1 минута). Инвалидация при POST решает эту проблему.

---

### Шаг 4 - Проверка rate limiting

Отправляем 35 запросов подряд к поиску по имени:

```bash
for i in $(seq 1 35); do
  CODE=$(curl -s -o /dev/null -w "%{http_code}" \
    "http://localhost:8080/api/v1/users?name=Ivan" \
    -H "Authorization: Bearer 4053890b3016014db3994469adaa688a")
  echo "Запрос $i: HTTP $CODE"
done
```

Ответ:
```
Запрос 1: HTTP 200
Запрос 2: HTTP 200
Запрос 3: HTTP 200
Запрос 4: HTTP 200
Запрос 5: HTTP 200
Запрос 6: HTTP 200
Запрос 7: HTTP 200
Запрос 8: HTTP 200
Запрос 9: HTTP 200
Запрос 10: HTTP 200
Запрос 11: HTTP 200
Запрос 12: HTTP 200
Запрос 13: HTTP 200
Запрос 14: HTTP 200
Запрос 15: HTTP 200
Запрос 16: HTTP 200
Запрос 17: HTTP 200
Запрос 18: HTTP 200
Запрос 19: HTTP 200
Запрос 20: HTTP 200
Запрос 21: HTTP 200
Запрос 22: HTTP 200
Запрос 23: HTTP 200
Запрос 24: HTTP 200
Запрос 25: HTTP 200
Запрос 26: HTTP 200
Запрос 27: HTTP 200
Запрос 28: HTTP 200
Запрос 29: HTTP 200
Запрос 30: HTTP 200
Запрос 31: HTTP 429
Запрос 32: HTTP 429
Запрос 33: HTTP 429
Запрос 34: HTTP 429
Запрос 35: HTTP 429

```

**Вывод:** ровно с 31-го запроса сервер начинает возвращать `429 Too Many Requests`. Через 60 секунд счётчик сбрасывается и запросы снова проходят.

Заголовки при запросе:

```bash
curl -si "http://localhost:8080/api/v1/users?name=Ivan" \
  -H "Authorization: Bearer 4053890b3016014db3994469adaa688a" | grep -i "ratelimit"
```

Ответ:
```
X-RateLimit-Limit: 30
X-RateLimit-Remaining: 29
X-RateLimit-Reset: 1746356400
```

---

### Шаг 5 - Метрики через /metrics до и после нагрузки

Смотрю метрики сразу после запуска - всё на нуле:

```bash
curl -s http://localhost:8080/metrics
```

Ответ:
```
# HELP cache_hits_total Количество запросов из кеша
# TYPE cache_hits_total counter
cache_hits_total 0

# HELP cache_misses_total Количество промахов кеша (запросов в MongoDB)
# TYPE cache_misses_total counter
cache_misses_total 0

# HELP cache_requests_total Всего запросов через кеш
# TYPE cache_requests_total counter
cache_requests_total 0

# HELP cache_hit_rate Процент попаданий в кеш (0..1)
# TYPE cache_hit_rate gauge
cache_hit_rate 0

# HELP rate_limit_blocked_total Количество запросов заблокированных rate limiter
# TYPE rate_limit_blocked_total counter
rate_limit_blocked_total 0
```

Запускаю нагрузочный тест - 20 запросов на кеш и 35 на rate limit:

```bash
for i in $(seq 1 20); do
  curl -s "http://localhost:8080/api/v1/users?login=ivan2" -H "Authorization: Bearer a50d99e7c49ed7f048a6106a0963d0fb" > /dev/null
done

for i in $(seq 1 35); do
  curl -s "http://localhost:8080/api/v1/users?name=Ivan" -H "Authorization: Bearer a50d99e7c49ed7f048a6106a0963d0fb" > /dev/null
done
```

Смотрю метрики после:

```bash
curl -s http://localhost:8080/metrics
```

Ответ:
```
# HELP cache_hits_total Количество запросов из кеша
# TYPE cache_hits_total counter
cache_hits_total 19

# HELP cache_misses_total Количество промахов кеша (запросов в MongoDB)
# TYPE cache_misses_total counter
cache_misses_total 1

# HELP cache_requests_total Всего запросов через кеш
# TYPE cache_requests_total counter
cache_requests_total 20

# HELP cache_hit_rate Процент попаданий в кеш (0..1)
# TYPE cache_hit_rate gauge
cache_hit_rate 0.95

# HELP rate_limit_blocked_total Количество запросов заблокированных rate limiter
# TYPE rate_limit_blocked_total counter
rate_limit_blocked_total 5
```

**Вывод:** из 20 запросов 19 попали в кеш (только первый пошёл в MongoDB), hit rate 0.95. Из 35 запросов к поиску по имени 5 заблокировал rate limiter.

---

### Шаг 6 - Grafana

Сделала дашборд в Grafana с четырьмя панелями:
- **Эффективность кеша** — gauge, показывает hit rate от 0 до 1
- **Запросы из кеша** — график по времени, видно когда идёт нагрузка
- **Промахи кеша** — график, показывает сколько раз пришлось идти в MongoDB
- **Заблокировано rate limiter** — счётчик, сколько запросов получили 429

Запустила нагрузочный тест на кеш (20 запросов подряд) и rate limiter (35 запросов подряд), потом смотрю в Grafana.

До тестов - всё на нуле, сервис только поднялся:

![Grafana до теста](service_template/screenshots/before_grafana_screenshot.jpg)

После тестов - видно скачки на графиках и счётчики:

![Grafana после теста](service_template/screenshots/after_grafana_screenshot.jpg)

На скрине видно:
- **Эффективность кеша 0.971** - 97% запросов отдаётся из памяти, не идя в MongoDB
- **График "Запросы из кеша"** - резкий скачок когда запустила нагрузочный тест
- **Промахи кеша** - маленький всплеск в начале (пока кеш прогревался), потом почти ноль
- **Заблокировано rate limiter: 15** - столько запросов получили HTTP 429 за оба теста

---

## API endpoints

Все запросы, кроме `/register` и `/login`, требуют заголовок `Authorization: Bearer <token>`.

| Метод | URL | Описание | Оптимизация |
|-------|-----|----------|-------------|
| POST | `/api/v1/auth/register` | Создание пользователя | - |
| POST | `/api/v1/auth/login` | Вход, возвращает токен | - |
| POST | `/api/v1/auth/logout` | Выход | - |
| GET  | `/api/v1/users?login=` | Поиск по логину | Кеш 5 минут |
| GET  | `/api/v1/users?name=` | Поиск по имени/фамилии | Rate limit 30/мин |
| POST | `/api/v1/wall/{user_id}` | Добавить пост на стену | Инвалидирует кеш |
| GET  | `/api/v1/wall/{user_id}` | Загрузить стену | Кеш 1 минута |
| POST | `/api/v1/messages` | Отправить сообщение | - |
| GET  | `/api/v1/messages?user_id=` | Получить сообщения | - |

---

## Структура файлов

```
service_template/
├── performance_design.md    # Описание стратегий кеширования и rate limiting
├── schema_design.md         # Проектирование модели MongoDB
├── data.js                  # Тестовые данные MongoDB
├── queries.js               # CRUD-запросы
├── validation.js            # Схемы валидации
├── docker-compose.yml
├── Dockerfile
├── schema.sql               # Схема PostgreSQL
├── data.sql                 # Тестовые данные PostgreSQL
└── src/
    ├── Cache.hpp/.cpp       # In-memory кеш с TTL
    ├── RateLimiter.hpp/.cpp # Rate limiter (Fixed Window Counter)
    ├── Storage.hpp/.cpp     # Хранилище сессий
    ├── MongoHelper.hpp/.cpp # Обёртка над libmongoc
    ├── Auth/                # Регистрация, вход, выход
    ├── User/                # Поиск пользователей (кеш + rate limit)
    ├── Wall/                # Стена (кеш + инвалидация)
    └── Chat/                # Личные сообщения
```
