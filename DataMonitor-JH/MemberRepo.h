#pragma once
#include <string>
#include <vector>
#include <optional>
#include "Member.h"

class MemberRepository {
public:
    explicit MemberRepository(const std::string& filepath);

    Member                create(const std::string& name,
                                 const std::string& email,
                                 const std::string& phone,
                                 int age);

    std::vector<Member>   findAll()   const;
    std::optional<Member> findById(int id) const;
    std::vector<Member>   findByName(const std::string& keyword) const;

    bool update(const Member& m);
    bool remove(int id);

private:
    std::string         m_filepath;
    std::vector<Member> m_data;
    int                 m_nextId = 1;

    void        load();
    void        save() const;
    std::string nowIso8601() const;
};
