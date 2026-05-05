use("social_network");

// очищение коллекции перед вставкой для повторного запуска
db.users.deleteMany({});
db.wall_posts.deleteMany({});
db.chat_messages.deleteMany({});

//пользователи
db.users.insertMany([
  {
    login: "ivan_ivanov",
    password_hash: "hash_pass1",
    first_name: "Ivan",
    last_name: "Ivanov",
    age: 28,
    interests: ["sports", "music", "cinema"],
    created_at: new Date("2024-01-10"),
  },
  {
    login: "maria_petrova",
    password_hash: "hash_pass2",
    first_name: "Maria",
    last_name: "Petrova",
    age: 25,
    interests: ["reading", "travel"],
    created_at: new Date("2024-01-11"),
  },
  {
    login: "alex_sidorov",
    password_hash: "hash_pass3",
    first_name: "Alex",
    last_name: "Sidorov",
    age: 32,
    interests: ["programming", "games"],
    created_at: new Date("2024-01-12"),
  },
  {
    login: "elena_kozlova",
    password_hash: "hash_pass4",
    first_name: "Elena",
    last_name: "Kozlova",
    age: 27,
    interests: ["photography", "cooking", "sports"],
    created_at: new Date("2024-01-13"),
  },
  {
    login: "dmitry_novikov",
    password_hash: "hash_pass5",
    first_name: "Dmitry",
    last_name: "Novikov",
    age: 35,
    interests: ["history", "cinema"],
    created_at: new Date("2024-01-14"),
  },
  {
    login: "olga_morozova",
    password_hash: "hash_pass6",
    first_name: "Olga",
    last_name: "Morozova",
    age: 23,
    interests: ["dancing", "music"],
    created_at: new Date("2024-01-15"),
  },
  {
    login: "sergey_volkov",
    password_hash: "hash_pass7",
    first_name: "Sergey",
    last_name: "Volkov",
    age: 30,
    interests: ["sports", "travel", "cooking"],
    created_at: new Date("2024-01-16"),
  },
  {
    login: "natasha_smirnova",
    password_hash: "hash_pass8",
    first_name: "Natasha",
    last_name: "Smirnova",
    age: 26,
    interests: ["reading", "cinema", "music"],
    created_at: new Date("2024-01-17"),
  },
  {
    login: "andrey_lebedev",
    password_hash: "hash_pass9",
    first_name: "Andrey",
    last_name: "Lebedev",
    age: 29,
    interests: ["programming", "sports"],
    created_at: new Date("2024-01-18"),
  },
  {
    login: "yulia_zhukova",
    password_hash: "hash_pass10",
    first_name: "Yulia",
    last_name: "Zhukova",
    age: 22,
    interests: ["photography", "travel"],
    created_at: new Date("2024-01-19"),
  },
  {
    login: "pavel_frolov",
    password_hash: "hash_pass11",
    first_name: "Pavel",
    last_name: "Frolov",
    age: 31,
    interests: ["games", "cinema"],
    created_at: new Date("2024-01-20"),
  },
  {
    login: "anna_popova",
    password_hash: "hash_pass12",
    first_name: "Anna",
    last_name: "Popova",
    age: 24,
    interests: ["dancing", "reading", "photography"],
    created_at: new Date("2024-01-21"),
  },
]);

// получение id пользователей для связей
const ivan = db.users.findOne({ login: "ivan_ivanov" })._id;
const maria = db.users.findOne({ login: "maria_petrova" })._id;
const alex = db.users.findOne({ login: "alex_sidorov" })._id;
const elena = db.users.findOne({ login: "elena_kozlova" })._id;
const dmitry = db.users.findOne({ login: "dmitry_novikov" })._id;
const olga = db.users.findOne({ login: "olga_morozova" })._id;
const sergey = db.users.findOne({ login: "sergey_volkov" })._id;
const natasha = db.users.findOne({ login: "natasha_smirnova" })._id;
const andrey = db.users.findOne({ login: "andrey_lebedev" })._id;
const yulia = db.users.findOne({ login: "yulia_zhukova" })._id;

// записи стены
db.wall_posts.insertMany([
  {
    owner_id: ivan,
    author_id: ivan,
    content: "Hello everyone! This is my first wall post.",
    created_at: new Date("2024-02-01T10:00:00Z"),
  },
  {
    owner_id: ivan,
    author_id: maria,
    content: "Ivan, happy birthday!",
    created_at: new Date("2024-02-15T12:00:00Z"),
  },
  {
    owner_id: ivan,
    author_id: alex,
    content: "Stopped by your wall. How are you?",
    created_at: new Date("2024-02-16T09:30:00Z"),
  },
  {
    owner_id: maria,
    author_id: maria,
    content: "Great weather today, going for a walk!",
    created_at: new Date("2024-03-01T14:00:00Z"),
  },
  {
    owner_id: maria,
    author_id: ivan,
    content: "Maria, when are we meeting?",
    created_at: new Date("2024-03-02T08:00:00Z"),
  },
  {
    owner_id: maria,
    author_id: elena,
    content: "Hey Maria! Long time no see.",
    created_at: new Date("2024-03-03T20:00:00Z"),
  },
  {
    owner_id: alex,
    author_id: alex,
    content: "Just got back from vacation. It was great!",
    created_at: new Date("2024-03-10T16:00:00Z"),
  },
  {
    owner_id: alex,
    author_id: dmitry,
    content: "Alex, tell me about your vacation!",
    created_at: new Date("2024-03-11T10:00:00Z"),
  },
  {
    owner_id: elena,
    author_id: elena,
    content: "New job, new opportunities!",
    created_at: new Date("2024-03-15T09:00:00Z"),
  },
  {
    owner_id: elena,
    author_id: olga,
    content: "Elena, congrats on the new job!",
    created_at: new Date("2024-03-15T11:00:00Z"),
  },
  {
    owner_id: dmitry,
    author_id: dmitry,
    content: "The weather today is just wonderful.",
    created_at: new Date("2024-04-01T13:00:00Z"),
  },
  {
    owner_id: olga,
    author_id: sergey,
    content: "Olga, your post was really interesting!",
    created_at: new Date("2024-04-05T17:00:00Z"),
  },
]);

// сообщения
db.chat_messages.insertMany([
  {
    sender_id: ivan,
    receiver_id: maria,
    text: "Hey Maria! How are you?",
    created_at: new Date("2024-04-01T10:00:00Z"),
  },
  {
    sender_id: maria,
    receiver_id: ivan,
    text: "Hey Ivan! All good, thanks.",
    created_at: new Date("2024-04-01T10:05:00Z"),
  },
  {
    sender_id: ivan,
    receiver_id: maria,
    text: "Any plans for the weekend?",
    created_at: new Date("2024-04-01T10:10:00Z"),
  },
  {
    sender_id: maria,
    receiver_id: ivan,
    text: "Yeah, thinking about going to an exhibition.",
    created_at: new Date("2024-04-01T10:15:00Z"),
  },
  {
    sender_id: alex,
    receiver_id: elena,
    text: "Elena, have you seen the new movie?",
    created_at: new Date("2024-04-02T15:00:00Z"),
  },
  {
    sender_id: elena,
    receiver_id: alex,
    text: "Not yet, is it worth it?",
    created_at: new Date("2024-04-02T15:10:00Z"),
  },
  {
    sender_id: alex,
    receiver_id: elena,
    text: "Definitely! Highly recommend it.",
    created_at: new Date("2024-04-02T15:15:00Z"),
  },
  {
    sender_id: dmitry,
    receiver_id: olga,
    text: "Hey Olga! When can we meet?",
    created_at: new Date("2024-04-03T09:00:00Z"),
  },
  {
    sender_id: olga,
    receiver_id: dmitry,
    text: "Hi! Maybe Friday evening?",
    created_at: new Date("2024-04-03T09:30:00Z"),
  },
  {
    sender_id: dmitry,
    receiver_id: olga,
    text: "Perfect, it's a deal!",
    created_at: new Date("2024-04-03T09:35:00Z"),
  },
  {
    sender_id: sergey,
    receiver_id: natasha,
    text: "Natasha, thanks for the help!",
    created_at: new Date("2024-04-04T11:00:00Z"),
  },
  {
    sender_id: andrey,
    receiver_id: yulia,
    text: "Yulia, happy holidays!",
    created_at: new Date("2024-04-05T08:00:00Z"),
  },
]);

print("Данные успешно загружены!");
print("users:" + db.users.countDocuments());
print("wall_posts:" + db.wall_posts.countDocuments());
print("chat_messages:" + db.chat_messages.countDocuments());
