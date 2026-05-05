use("social_network");

db.users.drop();

db.createCollection("users", {
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: ["login", "password_hash", "first_name", "last_name", "created_at"],
      properties: {
        login: {
          bsonType: "string",
          minLength: 3,
          maxLength: 64,
          pattern: "^[a-zA-Z0-9_]+$",
        },
        password_hash: {
          bsonType: "string",
          minLength: 1,
        },
        first_name: {
          bsonType: "string",
          minLength: 1,
          maxLength: 128,
        },
        last_name: {
          bsonType: "string",
          minLength: 1,
          maxLength: 128,
        },
        age: {
          bsonType: "int",
          minimum: 1,
          maximum: 150,
        },
        interests: {
          bsonType: "array",
          items: { bsonType: "string" },
        },
        created_at: {
          bsonType: "date",
        },
      },
    },
  },
  validationAction: "error",
});

// валидный пользователь — должно пройти
try {
  db.users.insertOne({
    login: "valid_user",
    password_hash: "hash123",
    first_name: "Валидный",
    last_name: "Пользователь",
    age: 25,
    interests: ["кино"],
    created_at: new Date(),
  });
  print("вставка прошла");
} catch (e) {
  print("ошибка: " + e.message);
}

// пустой логин
try {
  db.users.insertOne({
    login: "",
    password_hash: "hash",
    first_name: "Иван",
    last_name: "Иванов",
    created_at: new Date(),
  });
  print("должна была упасть");
} catch (e) {
  print("ожидаемая ошибка (пустой логин): " + e.message.substring(0, 60));
}

// нет обязательного поля
try {
  db.users.insertOne({
    login: "no_name",
    password_hash: "hash",
    last_name: "Иванов",
    created_at: new Date(),
  });
  print("должна была упасть");
} catch (e) {
  print("ожидаемая ошибка (нет first_name): " + e.message.substring(0, 60));
}

// логин с недопустимыми символами
try {
  db.users.insertOne({
    login: "user@mail",
    password_hash: "hash",
    first_name: "Иван",
    last_name: "Иванов",
    created_at: new Date(),
  });
  print("должна была упасть");
} catch (e) {
  print("ожидаемая ошибка (символы в логине): " + e.message.substring(0, 60));
}

// возраст вне диапазона
try {
  db.users.insertOne({
    login: "old_user",
    password_hash: "hash",
    first_name: "Иван",
    last_name: "Иванов",
    age: 200,
    created_at: new Date(),
  });
  print("должна была упасть");
} catch (e) {
  print("ожидаемая ошибка (возраст > 150): " + e.message.substring(0, 60));
}

db.wall_posts.drop();

db.createCollection("wall_posts", {
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: ["owner_id", "author_id", "content", "created_at"],
      properties: {
        owner_id: {
          bsonType: "objectId",
        },
        author_id: {
          bsonType: "objectId",
        },
        content: {
          bsonType: "string",
          minLength: 1,
        },
        created_at: {
          bsonType: "date",
        },
      },
    },
  },
  validationAction: "error",
});

// пост без контента
try {
  db.wall_posts.insertOne({
    owner_id: new ObjectId(),
    author_id: new ObjectId(),
    content: "",
    created_at: new Date(),
  });
  print("должна была упасть");
} catch (e) {
  print("ожидаемая ошибка (пустой контент): " + e.message.substring(0, 60));
}

db.chat_messages.drop();

db.createCollection("chat_messages", {
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: ["sender_id", "receiver_id", "text", "created_at"],
      properties: {
        sender_id: {
          bsonType: "objectId",
        },
        receiver_id: {
          bsonType: "objectId",
        },
        text: {
          bsonType: "string",
          minLength: 1,
        },
        created_at: {
          bsonType: "date",
        },
      },
    },
  },
  validationAction: "error",
});

// сообщение без текста
try {
  db.chat_messages.insertOne({
    sender_id: new ObjectId(),
    receiver_id: new ObjectId(),
    text: "",
    created_at: new Date(),
  });
  print("должна была упасть");
} catch (e) {
  print("ожидаемая ошибка (пустой текст): " + e.message.substring(0, 60));
}
