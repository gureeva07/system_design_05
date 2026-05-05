#include <userver/utest/utest.hpp>

#include "../Storage.hpp"

UTEST(WallStorage, AddWallPostReturnsPositiveId) {
  auto& storage = social::Storage::Instance();

  // Создаём владельца стены
  social::User owner;
  owner.login = "sn_wall_owner_1";
  owner.password_hash = "pwd";
  owner.first_name = "Владелец";
  owner.last_name = "Стены";
  int owner_id = storage.AddUser(owner);

  // Создаём автора
  social::User author;
  author.login = "sn_wall_author_1";
  author.password_hash = "pwd";
  author.first_name = "Автор";
  author.last_name = "Поста";
  int author_id = storage.AddUser(author);

  social::WallPost p;
  p.owner_id = owner_id;
  p.author_id = author_id;
  p.content = "Привет всем!";
  p.created_at = "2024-03-15T12:00:00";

  int id = storage.AddWallPost(p);
  EXPECT_GT(id, 0);
}

UTEST(WallStorage, AddWallPostIdsIncrement) {
  auto& storage = social::Storage::Instance();

  social::User u;
  u.login = "sn_wall_inc_user";
  u.password_hash = "pwd";
  u.first_name = "А";
  u.last_name = "Б";
  int uid = storage.AddUser(u);

  social::WallPost p1;
  p1.owner_id = uid;
  p1.author_id = uid;
  p1.content = "Пост 1";

  social::WallPost p2;
  p2.owner_id = uid;
  p2.author_id = uid;
  p2.content = "Пост 2";

  int id1 = storage.AddWallPost(p1);
  int id2 = storage.AddWallPost(p2);
  EXPECT_GT(id2, id1);
}

UTEST(WallStorage, GetWallPostsReturnsAllForOwner) {
  auto& storage = social::Storage::Instance();

  social::User u;
  u.login = "sn_wall_getall";
  u.password_hash = "pwd";
  u.first_name = "Получить";
  u.last_name = "Все";
  int uid = storage.AddUser(u);

  social::WallPost p1;
  p1.owner_id = uid;
  p1.author_id = uid;
  p1.content = "Запись 1";
  storage.AddWallPost(p1);

  social::WallPost p2;
  p2.owner_id = uid;
  p2.author_id = uid;
  p2.content = "Запись 2";
  storage.AddWallPost(p2);

  auto posts = storage.GetWallPosts(uid);
  EXPECT_GE(posts.size(), 2u);

  for (const auto& post : posts) {
    EXPECT_EQ(post.owner_id, uid);
  }
}

UTEST(WallStorage, GetWallPostsEmptyForNewUser) {
  auto& storage = social::Storage::Instance();

  social::User u;
  u.login = "sn_wall_empty";
  u.password_hash = "pwd";
  u.first_name = "Пустая";
  u.last_name = "Стена";
  int uid = storage.AddUser(u);

  auto posts = storage.GetWallPosts(uid);
  EXPECT_TRUE(posts.empty());
}

UTEST(WallStorage, GetWallPostsIsolatedBetweenUsers) {
  auto& storage = social::Storage::Instance();

  social::User u1;
  u1.login = "sn_wall_iso_1";
  u1.password_hash = "pwd";
  u1.first_name = "Первый";
  u1.last_name = "Пользователь";
  int uid1 = storage.AddUser(u1);

  social::User u2;
  u2.login = "sn_wall_iso_2";
  u2.password_hash = "pwd";
  u2.first_name = "Второй";
  u2.last_name = "Пользователь";
  int uid2 = storage.AddUser(u2);

  // Добавляем пост только на стену первого
  social::WallPost p;
  p.owner_id = uid1;
  p.author_id = uid1;
  p.content = "Пост только для первого";
  storage.AddWallPost(p);

  auto posts1 = storage.GetWallPosts(uid1);
  auto posts2 = storage.GetWallPosts(uid2);

  EXPECT_GE(posts1.size(), 1u);
  EXPECT_TRUE(posts2.empty());
}

UTEST(WallStorage, GetWallPostsContentPreserved) {
  auto& storage = social::Storage::Instance();

  social::User u;
  u.login = "sn_wall_content";
  u.password_hash = "pwd";
  u.first_name = "Контент";
  u.last_name = "Проверка";
  int uid = storage.AddUser(u);

  social::WallPost p;
  p.owner_id = uid;
  p.author_id = uid;
  p.content = "Уникальный текст поста для проверки";
  p.created_at = "2024-06-01T10:00:00";
  storage.AddWallPost(p);

  auto posts = storage.GetWallPosts(uid);
  ASSERT_GE(posts.size(), 1u);

  bool found = false;
  for (const auto& post : posts) {
    if (post.content == "Уникальный текст поста для проверки") {
      EXPECT_EQ(post.owner_id, uid);
      EXPECT_EQ(post.author_id, uid);
      EXPECT_EQ(post.created_at, "2024-06-01T10:00:00");
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

UTEST(WallStorage, AuthorCanPostOnAnotherUsersWall) {
  auto& storage = social::Storage::Instance();

  social::User owner;
  owner.login = "sn_wall_other_owner";
  owner.password_hash = "pwd";
  owner.first_name = "Хозяин";
  owner.last_name = "Стены";
  int owner_id = storage.AddUser(owner);

  social::User visitor;
  visitor.login = "sn_wall_other_visitor";
  visitor.password_hash = "pwd";
  visitor.first_name = "Гость";
  visitor.last_name = "Гостев";
  int visitor_id = storage.AddUser(visitor);

  social::WallPost p;
  p.owner_id = owner_id;
  p.author_id = visitor_id;  // гость пишет на стену хозяина
  p.content = "Привет от гостя!";
  storage.AddWallPost(p);

  auto posts = storage.GetWallPosts(owner_id);
  ASSERT_GE(posts.size(), 1u);

  bool found = false;
  for (const auto& post : posts) {
    if (post.content == "Привет от гостя!") {
      EXPECT_EQ(post.author_id, visitor_id);
      EXPECT_EQ(post.owner_id, owner_id);
      found = true;
    }
  }
  EXPECT_TRUE(found);
}
