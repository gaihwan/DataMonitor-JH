#include "MemberRepo.h"
#include <fstream>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

MemberRepository::MemberRepository(const std::string& filepath)
    : m_filepath(filepath)
{
    load();
}

void MemberRepository::load()
{
    std::ifstream ifs(m_filepath);
    if (!ifs.is_open()) {
        m_data.clear();
        m_nextId = 1;
        return;
    }

    try {
        json j;
        ifs >> j;
        m_data = j.get<std::vector<Member>>();
    } catch (const std::exception& e) {
        std::cerr << "[MemberRepository] Warning: failed to parse " << m_filepath
                  << " (" << e.what() << "). Starting with empty data." << std::endl;
        m_data.clear();
    }

    m_nextId = 1;
    for (const auto& m : m_data)
        if (m.id >= m_nextId) m_nextId = m.id + 1;
}

void MemberRepository::save() const
{
    std::ofstream ofs(m_filepath);
    if (!ofs.is_open()) {
        std::cerr << "[MemberRepository] Warning: cannot write to " << m_filepath << std::endl;
        return;
    }
    json j = m_data;
    ofs << j.dump(2);
}

std::string MemberRepository::nowIso8601() const
{
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

Member MemberRepository::create(const std::string& name,
                                 const std::string& email,
                                 const std::string& phone,
                                 int age)
{
    Member m;
    m.id         = m_nextId++;
    m.name       = name;
    m.email      = email;
    m.phone      = phone;
    m.age        = age;
    m.created_at = nowIso8601();

    m_data.push_back(m);
    save();
    return m;
}

std::vector<Member> MemberRepository::findAll() const
{
    return m_data;
}

std::optional<Member> MemberRepository::findById(int id) const
{
    auto it = std::find_if(m_data.begin(), m_data.end(),
                           [id](const Member& m){ return m.id == id; });
    if (it == m_data.end()) return std::nullopt;
    return *it;
}

std::vector<Member> MemberRepository::findByName(const std::string& keyword) const
{
    std::vector<Member> result;
    for (const auto& m : m_data)
        if (m.name.find(keyword) != std::string::npos)
            result.push_back(m);
    return result;
}

bool MemberRepository::update(const Member& updated)
{
    auto it = std::find_if(m_data.begin(), m_data.end(),
                           [&](const Member& m){ return m.id == updated.id; });
    if (it == m_data.end()) return false;

    *it = updated;
    save();
    return true;
}

bool MemberRepository::remove(int id)
{
    auto it = std::find_if(m_data.begin(), m_data.end(),
                           [id](const Member& m){ return m.id == id; });
    if (it == m_data.end()) return false;

    m_data.erase(it);
    save();
    return true;
}
