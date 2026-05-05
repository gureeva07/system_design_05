#include <userver/utest/utest.hpp>

#include "../Storage.hpp"

UTEST(UserStorage, AddUserReturnsPositiveId) {
  auto& storage = social::Storage::Instance();

  social::User u;
  u.login = "sn_add_1";
  u.password_hash = "pass";
  u.first_name = "Иван";
  u.last_name = "Иванов";

  int id = storage.AddUser(u);
  EXPECT_GT(id, 0);
}

UTEST(UserStorage, AddUserIdsIncrement) {
  auto& storage = social::Storage::Instance();

  social::User u1;
  u1.login = "sn_inc_1";
  u1.password_hash = "p1";
  u1.first_name = "А";
  u1.last_name = "Б";

  social::User u2;
  u2.login = "sn_inc_2";
  u2.password_hash = "p2";
  u2.first_name = "В";
  u2.last_name = "Г";

  int id1 = storage.AddUser(u1);
  int id2 = storage.AddUser(u2);
  EXPECT_GT(id2, id1);
}

UTEST(UserStorage, FindUserByLoginSuccess) {
  auto& storage = social::Storage::Instance();

  social::User u;
  u.login = "sn_find_login";
  u.password_hash = "secret";
  u.first_name = "Алиса";
  u.last_name = "Петрова";

  storage.AddUser(u);

  auto found = storage.FindUserByLogin("sn_find_login");
  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(found->login, "sn_find_login");
  EXPECT_EQ(found->first_name, "Алиса");
  EXPECT_EQ(found->last_name, "Петрова");
}

UTEST(UserStorage, FindUserByLoginNotFound) {
  auto& storage = social::Storage::Instance();

  auto found = storage.FindUserByLogin("sn_nonexistent_xyz");
  EXPECT_FALSE(found.has_value());
}

UTEST(UserStorage, GetUserByIdSuccess) {
  auto& storage = social::Storage::Instance();

  social::User u;
  u.login = "sn_getbyid";
  u.password_hash = "pwd";
  u.first_name = "Борис";
  u.last_name = "Сидоров";

  int id = storage.AddUser(u);

  auto found = storage.GetUser(id);
  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(found->id, id);
  EXPECT_EQ(found->login, "sn_getbyid");
}

UTEST(UserStorage, GetUserByIdNotFound) {
  auto& storage = social::Storage::Instance();

  auto found = storage.GetUser(999999);
  EXPECT_FALSE(found.has_value());
}

UTEST(UserStorage, FindUsersByNameMatchesFirstName) {
  auto& storage = social::Storage::Instance();

  social::User u;
  u.login = "sn_namesearch_1";
  u.password_hash = "pwd";
  u.first_name = "Uniquefirstxyz";
  u.last_name = "Обычная";

  storage.AddUser(u);

  auto results = storage.FindUsersByName("uniquefirstxyz");
  ASSERT_GE(results.size(), 1u);
  EXPECT_EQ(results[0].first_name, "Uniquefirstxyz");
}

UTEST(UserStorage, FindUsersByNameMatchesLastName) {
  auto& storage = social::Storage::Instance();

  social::User u;
  u.login = "sn_namesearch_2";
  u.password_hash = "pwd";
  u.first_name = "Обычное";
  u.last_name = "Uniquelastxyz";

  storage.AddUser(u);

  auto results = storage.FindUsersByName("uniquelastxyz");
  ASSERT_GE(results.size(), 1u);
  EXPECT_EQ(results[0].last_name, "Uniquelastxyz");
}

UTEST(UserStorage, FindUsersByNameCaseInsensitive) {
  auto& storage = social::Storage::Instance();

  social::User u;
  u.login = "sn_namesearch_3";
  u.password_hash = "pwd";
  u.first_name = "Mixedcase";
  u.last_name = "Testович";

  storage.AddUser(u);

  auto results = storage.FindUsersByName("MIXEDCASE");
  ASSERT_GE(results.size(), 1u);
  EXPECT_EQ(results[0].first_name, "Mixedcase");
}

UTEST(UserStorage, FindUsersByNameNoMatch) {
  auto& storage = social::Storage::Instance();

  auto results = storage.FindUsersByName("zzz_no_match_zzz");
  EXPECT_TRUE(results.empty());
}

UTEST(UserSession, CreateSessionReturnsToken) {
  auto& storage = social::Storage::Instance();

  social::User u;
  u.login = "sn_session_user";
  u.password_hash = "pwd";
  u.first_name = "С";
  u.last_name = "У";

  int uid = storage.AddUser(u);
  std::string token = storage.CreateSession(uid);

  EXPECT_FALSE(token.empty());
}

UTEST(UserSession, ValidateSessionSuccess) {
  auto& storage = social::Storage::Instance();

  social::User u;
  u.login = "sn_validate_user";
  u.password_hash = "pwd";
  u.first_name = "В";
  u.last_name = "У";

  int uid = storage.AddUser(u);
  std::string token = storage.CreateSession(uid);

  auto result = storage.ValidateSession(token);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, uid);
}

UTEST(UserSession, ValidateSessionInvalidToken) {
  auto& storage = social::Storage::Instance();

  auto result = storage.ValidateSession("invalid_token_xyz");
  EXPECT_FALSE(result.has_value());
}

UTEST(UserSession, DeleteSessionInvalidatesToken) {
  auto& storage = social::Storage::Instance();

  social::User u;
  u.login = "sn_delete_session";
  u.password_hash = "pwd";
  u.first_name = "Д";
  u.last_name = "С";

  int uid = storage.AddUser(u);
  std::string token = storage.CreateSession(uid);

  ASSERT_TRUE(storage.ValidateSession(token).has_value());

  storage.DeleteSession(token);

  EXPECT_FALSE(storage.ValidateSession(token).has_value());
}

UTEST(UserSession, TwoSessionsAreIndependent) {
  auto& storage = social::Storage::Instance();

  social::User u;
  u.login = "sn_two_sessions";
  u.password_hash = "pwd";
  u.first_name = "Т";
  u.last_name = "С";

  int uid = storage.AddUser(u);
  std::string token1 = storage.CreateSession(uid);
  std::string token2 = storage.CreateSession(uid);

  EXPECT_NE(token1, token2);

  storage.DeleteSession(token1);
  EXPECT_FALSE(storage.ValidateSession(token1).has_value());
  EXPECT_TRUE(storage.ValidateSession(token2).has_value());
}
