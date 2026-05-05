cmake_minimum_required(VERSION 3.12...3.31)
project(service_template CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DownloadUserver)

# Используем только postgresql — mongodb берём через системный libmongoc
find_package(userver COMPONENTS core postgresql QUIET)
if(NOT userver_FOUND)
  download_userver(TRY_DIR third_party/userver VERSION 2.8)
endif()

userver_setup_environment()

# Подключаем libmongoc через pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(MONGOC REQUIRED libmongoc-1.0)

# Common sources
add_library(${PROJECT_NAME}_objs OBJECT
    src/DTO.hpp
    src/JsonHelper.hpp
    src/JsonHelper.cpp
    src/Storage.hpp
    src/Storage.cpp
    src/Cache.hpp
    src/Cache.cpp
    src/RateLimiter.hpp
    src/RateLimiter.cpp
    src/MetricsHandler.hpp
    src/MetricsHandler.cpp
    src/MongoHelper.hpp
    src/MongoHelper.cpp
    src/Auth/AuthHandlers.hpp
    src/Auth/AuthHandlers.cpp
    src/AuthHelper.hpp
    src/User/UserHandlers.hpp
    src/User/UserHandlers.cpp
    src/Chat/ChatHandlers.hpp
    src/Chat/ChatHandlers.cpp
    src/Wall/WallHandlers.hpp
    src/Wall/WallHandlers.cpp
)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC userver::core userver::postgresql ${MONGOC_LIBRARIES})
target_include_directories(${PROJECT_NAME}_objs PUBLIC ${MONGOC_INCLUDE_DIRS})

# The Service
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

# Unit Tests
add_executable(${PROJECT_NAME}_unittest
)
target_link_libraries(${PROJECT_NAME}_unittest PRIVATE ${PROJECT_NAME}_objs userver::utest)
add_google_tests(${PROJECT_NAME}_unittest)
