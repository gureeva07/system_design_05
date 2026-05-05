#include <userver/utest/utest.hpp>

#include "../Storage.hpp"

UTEST(ChatStorage, AddMessageReturnsPositiveId) {
  auto& storage = social::Storage::Instance();

  social::User sender;
  sender.login = "sn_chat_sender_1";
  sender.password_hash = "pwd";
  sender.first_name = "Отправитель";
  sender.last_name = "Один";
  int sender_id = storage.AddUser(sender);

  social::User receiver;
  receiver.login = "sn_chat_receiver_1";
  receiver.password_hash = "pwd";
  receiver.first_name = "Получатель";
  receiver.last_name = "Один";
  int receiver_id = storage.AddUser(receiver);

  social::ChatMessage m;
  m.sender_id = sender_id;
  m.receiver_id = receiver_id;
  m.text = "Привет!";
  m.created_at = "2024-03-15T12:05:00";

  int id = storage.AddMessage(m);
  EXPECT_GT(id, 0);
}

UTEST(ChatStorage, AddMessageIdsIncrement) {
  auto& storage = social::Storage::Instance();

  social::User u1;
  u1.login = "sn_chat_inc_s";
  u1.password_hash = "pwd";
  u1.first_name = "А";
  u1.last_name = "Б";
  int uid1 = storage.AddUser(u1);

  social::User u2;
  u2.login = "sn_chat_inc_r";
  u2.password_hash = "pwd";
  u2.first_name = "В";
  u2.last_name = "Г";
  int uid2 = storage.AddUser(u2);

  social::ChatMessage m1;
  m1.sender_id = uid1;
  m1.receiver_id = uid2;
  m1.text = "Сообщение 1";

  social::ChatMessage m2;
  m2.sender_id = uid1;
  m2.receiver_id = uid2;
  m2.text = "Сообщение 2";

  int id1 = storage.AddMessage(m1);
  int id2 = storage.AddMessage(m2);
  EXPECT_GT(id2, id1);
}

UTEST(ChatStorage, GetMessagesIncludesIncoming) {
  auto& storage = social::Storage::Instance();

  social::User sender;
  sender.login = "sn_chat_in_sender";
  sender.password_hash = "pwd";
  sender.first_name = "Отп";
  sender.last_name = "Два";
  int sender_id = storage.AddUser(sender);

  social::User receiver;
  receiver.login = "sn_chat_in_receiver";
  receiver.password_hash = "pwd";
  receiver.first_name = "Пол";
  receiver.last_name = "Два";
  int receiver_id = storage.AddUser(receiver);

  social::ChatMessage m;
  m.sender_id = sender_id;
  m.receiver_id = receiver_id;
  m.text = "Входящее для получателя";
  storage.AddMessage(m);

  // Получатель должен видеть входящее сообщение
  auto messages = storage.GetMessagesForUser(receiver_id);
  EXPECT_GE(messages.size(), 1u);

  bool found = false;
  for (const auto& msg : messages) {
    if (msg.text == "Входящее для получателя") {
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

UTEST(ChatStorage, GetMessagesIncludesOutgoing) {
  auto& storage = social::Storage::Instance();

  social::User sender;
  sender.login = "sn_chat_out_sender";
  sender.password_hash = "pwd";
  sender.first_name = "Отп";
  sender.last_name = "Три";
  int sender_id = storage.AddUser(sender);

  social::User receiver;
  receiver.login = "sn_chat_out_receiver";
  receiver.password_hash = "pwd";
  receiver.first_name = "Пол";
  receiver.last_name = "Три";
  int receiver_id = storage.AddUser(receiver);

  social::ChatMessage m;
  m.sender_id = sender_id;
  m.receiver_id = receiver_id;
  m.text = "Исходящее от отправителя";
  storage.AddMessage(m);

  // Отправитель должен видеть исходящее сообщение
  auto messages = storage.GetMessagesForUser(sender_id);
  EXPECT_GE(messages.size(), 1u);

  bool found = false;
  for (const auto& msg : messages) {
    if (msg.text == "Исходящее от отправителя") {
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

UTEST(ChatStorage, GetMessagesEmptyForNewUser) {
  auto& storage = social::Storage::Instance();

  social::User u;
  u.login = "sn_chat_empty_user";
  u.password_hash = "pwd";
  u.first_name = "Новый";
  u.last_name = "Молчун";
  int uid = storage.AddUser(u);

  auto messages = storage.GetMessagesForUser(uid);
  EXPECT_TRUE(messages.empty());
}

UTEST(ChatStorage, GetMessagesContentPreserved) {
  auto& storage = social::Storage::Instance();

  social::User s;
  s.login = "sn_chat_content_s";
  s.password_hash = "pwd";
  s.first_name = "С";
  s.last_name = "О";
  int sid = storage.AddUser(s);

  social::User r;
  r.login = "sn_chat_content_r";
  r.password_hash = "pwd";
  r.first_name = "П";
  r.last_name = "О";
  int rid = storage.AddUser(r);

  social::ChatMessage m;
  m.sender_id = sid;
  m.receiver_id = rid;
  m.text = "Уникальный текст для проверки содержимого";
  m.created_at = "2024-05-10T08:30:00";
  storage.AddMessage(m);

  auto messages = storage.GetMessagesForUser(rid);
  bool found = false;
  for (const auto& msg : messages) {
    if (msg.text == "Уникальный текст для проверки содержимого") {
      EXPECT_EQ(msg.sender_id, sid);
      EXPECT_EQ(msg.receiver_id, rid);
      EXPECT_EQ(msg.created_at, "2024-05-10T08:30:00");
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

UTEST(ChatStorage, MessagesIsolatedBetweenConversations) {
  auto& storage = social::Storage::Instance();

  social::User u1;
  u1.login = "sn_chat_iso_u1";
  u1.password_hash = "pwd";
  u1.first_name = "Первый";
  u1.last_name = "Изо";
  int uid1 = storage.AddUser(u1);

  social::User u2;
  u2.login = "sn_chat_iso_u2";
  u2.password_hash = "pwd";
  u2.first_name = "Второй";
  u2.last_name = "Изо";
  int uid2 = storage.AddUser(u2);

  social::User u3;
  u3.login = "sn_chat_iso_u3";
  u3.password_hash = "pwd";
  u3.first_name = "Третий";
  u3.last_name = "Изо";
  int uid3 = storage.AddUser(u3);

  // u1 пишет u2
  social::ChatMessage m12;
  m12.sender_id = uid1;
  m12.receiver_id = uid2;
  m12.text = "Сообщение от 1 к 2";
  storage.AddMessage(m12);

  // u3 не участвует в переписке
  auto msgs3 = storage.GetMessagesForUser(uid3);
  for (const auto& msg : msgs3) {
    // u3 не должен видеть переписку u1 и u2
    EXPECT_FALSE(msg.text == "Сообщение от 1 к 2");
  }
}

UTEST(ChatStorage, MultipleMessagesInConversation) {
  auto& storage = social::Storage::Instance();

  social::User s;
  s.login = "sn_chat_multi_s";
  s.password_hash = "pwd";
  s.first_name = "Много";
  s.last_name = "Отправитель";
  int sid = storage.AddUser(s);

  social::User r;
  r.login = "sn_chat_multi_r";
  r.password_hash = "pwd";
  r.first_name = "Много";
  r.last_name = "Получатель";
  int rid = storage.AddUser(r);

  for (int i = 0; i < 3; ++i) {
    social::ChatMessage m;
    m.sender_id = sid;
    m.receiver_id = rid;
    m.text = "Сообщение " + std::to_string(i);
    storage.AddMessage(m);
  }

  auto messages = storage.GetMessagesForUser(rid);
  EXPECT_GE(messages.size(), 3u);
}
