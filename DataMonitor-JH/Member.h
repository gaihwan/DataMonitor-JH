#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct Member {
    int         id         = 0;
    std::string name;
    std::string email;
    std::string phone;
    int         age        = 0;
    std::string created_at;
};

inline void to_json(nlohmann::json& j, const Member& m) {
    j = nlohmann::json{
        {"id",         m.id},
        {"name",       m.name},
        {"email",      m.email},
        {"phone",      m.phone},
        {"age",        m.age},
        {"created_at", m.created_at}
    };
}

inline void from_json(const nlohmann::json& j, Member& m) {
    j.at("id").get_to(m.id);
    j.at("name").get_to(m.name);
    j.at("email").get_to(m.email);
    j.at("phone").get_to(m.phone);
    j.at("age").get_to(m.age);
    j.at("created_at").get_to(m.created_at);
}
