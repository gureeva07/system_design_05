use("social_network");

// создание нового пользователя
db.users.insertOne({
  login: "test_user",
  password_hash: "hash_test",
  first_name: "Test",
  last_name: "User",
  created_at: new Date(),
});
print("создан пользователь:");
printjson(db.users.findOne({ login: "test_user" }));

// поиск по логину
printjson(db.users.findOne({ login: { $eq: "ivan_ivanov" } }));

// поиск по маске имени/фамилии
printjson(db.users.find({
  $or: [
    { first_name: { $regex: "ivan", $options: "i" } },
    { last_name: { $regex: "ivan", $options: "i" } },
  ],
}).toArray());

// добавление записи на стену
const ivan = db.users.findOne({ login: "ivan_ivanov" })._id;
const maria = db.users.findOne({ login: "maria_petrova" })._id;

db.wall_posts.insertOne({
  owner_id: ivan,
  author_id: maria,
  content: "Hey Ivan, greetings from Maria!",
  created_at: new Date(),
});
printjson(db.wall_posts.findOne({ content: "Hey Ivan, greetings from Maria!" }));

// загрузка стены пользователя с данными автора через $lookup
const wall = db.wall_posts.aggregate([
  { $match: { owner_id: ivan } },
  {
    $lookup: {
      from: "users",
      localField: "author_id",
      foreignField: "_id",
      as: "author",
    },
  },
  { $unwind: "$author" },
  {
    $project: {
      content: 1,
      created_at: 1,
      "author.login": 1,
      "author.first_name": 1,
      "author.last_name": 1,
    },
  },
  { $sort: { created_at: -1 } },
]).toArray();
printjson(wall);

// отправка сообщения
const alex = db.users.findOne({ login: "alex_sidorov" })._id;

db.chat_messages.insertOne({
  sender_id: ivan,
  receiver_id: alex,
  text: "Hey Alex! How are you?",
  created_at: new Date(),
});

// список сообщений ивана
printjson(db.chat_messages.find({
  $or: [{ sender_id: ivan }, { receiver_id: ivan }],
}).sort({ created_at: -1 }).toArray());

// $ne - все кроме ивана
const notIvan = db.users.find({ login: { $ne: "ivan_ivanov" } }).toArray();
print("кол-во: " + notIvan.length);

// $in - несколько пользователей сразу
printjson(db.users.find({
  login: { $in: ["ivan_ivanov", "maria_petrova", "alex_sidorov"] }
}).toArray().map(u => u.login));

// $gt - посты после определённой даты
const recentPosts = db.wall_posts.find({ created_at: { $gt: new Date("2024-03-01") } }).toArray();
print("постов после 01.03.2024: " + recentPosts.length);

// $lt - пользователи младше 30 лет
printjson(db.users.find({ age: { $lt: 30 } }).toArray().map(u => u.login + " (" + u.age + ")"));

// поиск по интересам через $in для массива
printjson(db.users.find({ interests: { $in: ["sports"] } }).toArray().map(u => u.login));

// $and - чужие посты на стене ивана
printjson(db.wall_posts.find({
  $and: [{ owner_id: ivan }, { author_id: { $ne: ivan } }],
}).toArray().map(p => p.content));

// обновление поста
db.wall_posts.updateOne(
  { content: "Hey Ivan, greetings from Maria!" },
  { $set: { content: "Hey Ivan, big greetings from Maria!", updated_at: new Date() } }
);
printjson(db.wall_posts.findOne({ author_id: maria, owner_id: ivan }));

// удаление тестовых данных
db.users.deleteOne({ login: "test_user" });
db.wall_posts.deleteOne({ content: "Hey Ivan, big greetings from Maria!" });
db.chat_messages.deleteOne({ text: "Hey Alex! How are you?" });
