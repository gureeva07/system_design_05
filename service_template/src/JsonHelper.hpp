#pragma once
#include "DTO.hpp"

#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>

namespace social {

userver::formats::json::Value UserToJson(const User& u);
userver::formats::json::Value WallPostToJson(const WallPost& p);
userver::formats::json::Value ChatMessageToJson(const ChatMessage& m);

}  // namespace social
