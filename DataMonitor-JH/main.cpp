#include <iostream>
#include <string>
#include <limits>
#include "MemberRepo.h"

static void printMember(const Member& m)
{
    std::cout << "  [" << m.id << "] "
              << m.name << " | " << m.email
              << " | " << m.phone
              << " | age:" << m.age
              << " | " << m.created_at
              << std::endl;
}

static void printAll(MemberRepository& repo)
{
    auto list = repo.findAll();
    if (list.empty()) {
        std::cout << "  (no members)\n";
        return;
    }
    for (const auto& m : list) printMember(m);
}

static void doCreate(MemberRepository& repo)
{
    std::string name, email, phone;
    int age;

    std::cout << "Name  : "; std::getline(std::cin, name);
    std::cout << "Email : "; std::getline(std::cin, email);
    std::cout << "Phone : "; std::getline(std::cin, phone);
    std::cout << "Age   : "; std::cin >> age;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    Member created = repo.create(name, email, phone, age);
    std::cout << "Created: ";
    printMember(created);
}

static void doReadAll(MemberRepository& repo)
{
    std::cout << "-- All Members --\n";
    printAll(repo);
}

static void doReadById(MemberRepository& repo)
{
    int id;
    std::cout << "ID: "; std::cin >> id;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    auto opt = repo.findById(id);
    if (!opt) { std::cout << "Not found.\n"; return; }
    printMember(*opt);
}

static void doSearch(MemberRepository& repo)
{
    std::string keyword;
    std::cout << "Name keyword: "; std::getline(std::cin, keyword);

    auto results = repo.findByName(keyword);
    if (results.empty()) { std::cout << "No results.\n"; return; }
    for (const auto& m : results) printMember(m);
}

static void doUpdate(MemberRepository& repo)
{
    int id;
    std::cout << "ID to update: "; std::cin >> id;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    auto opt = repo.findById(id);
    if (!opt) { std::cout << "Not found.\n"; return; }

    Member m = *opt;
    std::cout << "Current: "; printMember(m);

    std::string buf;
    std::cout << "New name  [" << m.name  << "]: "; std::getline(std::cin, buf);
    if (!buf.empty()) m.name = buf;

    std::cout << "New email [" << m.email << "]: "; std::getline(std::cin, buf);
    if (!buf.empty()) m.email = buf;

    std::cout << "New phone [" << m.phone << "]: "; std::getline(std::cin, buf);
    if (!buf.empty()) m.phone = buf;

    std::cout << "New age   [" << m.age   << "]: "; std::getline(std::cin, buf);
    if (!buf.empty()) m.age = std::stoi(buf);

    repo.update(m);
    std::cout << "Updated: "; printMember(m);
}

static void doDelete(MemberRepository& repo)
{
    int id;
    std::cout << "ID to delete: "; std::cin >> id;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (repo.remove(id))
        std::cout << "Deleted id=" << id << "\n";
    else
        std::cout << "Not found.\n";
}

int main()
{
    MemberRepository repo("members.json");

    while (true) {
        std::cout << "\n===== Member CRUD =====\n"
                  << " 1. List all\n"
                  << " 2. Find by ID\n"
                  << " 3. Search by name\n"
                  << " 4. Create\n"
                  << " 5. Update\n"
                  << " 6. Delete\n"
                  << " 0. Exit\n"
                  << "Select: ";

        int choice;
        if (!(std::cin >> choice)) break;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
        case 1: doReadAll(repo);   break;
        case 2: doReadById(repo);  break;
        case 3: doSearch(repo);    break;
        case 4: doCreate(repo);    break;
        case 5: doUpdate(repo);    break;
        case 6: doDelete(repo);    break;
        case 0: return 0;
        default: std::cout << "Invalid.\n";
        }
    }
    return 0;
}
