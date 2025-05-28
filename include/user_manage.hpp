#ifndef USER_MANAGE_HPP
#define USER_MANAGE_HPP

#include "config.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <utility>
#include <tuple>

const std::string user_file = "/home/jqz/SMART_CMS/users.txt";
const std::string teacher_file = "/home/jqz/SMART_CMS/teacher_info.txt";
const std::string relationship_file = "/home/jqz/SMART_CMS/relationships.txt";
const std::string record_file = "/home/jqz/SMART_CMS/student_records.txt";

std::string hashPasswd(const std::string& password, const std::string& salt);
std::string generateSalt(size_t length = 16);
std::string binaryToHex(const std::string& binary);

class User {
protected:
    std::string user_type;
    std::string user_name;
    std::string hash_passwd;
    std::string salt;
public:
    friend std::unique_ptr<User> fromFile(const std::string&);
    User(const std::string type, const std::string& name, const std::string& passwd)
        : user_type(type), user_name(name), salt(generateSalt()) {
        hash_passwd = binaryToHex(hashPasswd(passwd, salt));
    }

    std::string GetType() const { return user_type; }
    std::string Gethash_passwd() const { return hash_passwd; }
    std::string Getsalt() const { return salt; }
    std::string GetName() const { return user_name; }

    void ChangeType(const std::string& type) { user_type = type; }
    void ChangeName(const std::string& name) { user_name = name; }
    void ChangeHash(const std::string& hash) { hash_passwd = hash; }
    void ChangeSalt(const std::string& salt_) { salt = salt_; }
    virtual ~User() = default;

    virtual std::string Tofilestring();
};

class Teacher : public User {
public:
    Teacher() : User("1", "", "") {}
    Teacher(const std::string& name) : User("1", name, "") {}
    Teacher(std::string education_, std::string character_, std::vector<std::string> subjects_,
            uint16_t price_min_, uint16_t price_high_, std::vector<std::string> allow_location_,
            std::vector<std::pair<std::string, std::pair<int, int>>> available_times_)
        : User("1", "", ""), education(education_), character(character_), subjects(subjects_),
          price_min(price_min_), price_high(price_high_), allow_location(allow_location_),
          available_times(available_times_) {}

    std::string education;
    std::string character;
    std::vector<std::string> subjects;
    uint16_t price_min, price_high;
    std::vector<std::string> allow_location;
    std::vector<std::pair<std::string, std::pair<int, int>>> available_times;

    void add_available_time(const std::string& weekday, int time_begin, int time_end) {
        available_times.emplace_back(weekday, std::make_pair(time_begin, time_end));
    }

    void remove_available_time(const std::string& weekday, int start, int end) {
        std::vector<std::pair<std::string, std::pair<int, int>>> new_times;
        for (const auto& time : available_times) {
            if (time.first != weekday) {
                new_times.push_back(time);
                continue;
            }
            int t_start = time.second.first;
            int t_end = time.second.second;
            if (end <= t_start || start >= t_end) {
                new_times.push_back(time);
                continue;
            }
            if (start > t_start) {
                new_times.emplace_back(weekday, std::make_pair(t_start, start));
            }
            if (end < t_end) {
                new_times.emplace_back(weekday, std::make_pair(end, t_end));
            }
        }
        available_times = new_times;
    }

    bool has_time_conflict(const std::string& weekday, int start, int end) const {
        for (const auto& time : available_times) {
            if (time.first != weekday) continue;
            int t_start = time.second.first;
            int t_end = time.second.second;
            if (start < t_end && end > t_start) {
                return true;
            }
        }
        return false;
    }

    std::string ToTeachfilestring();

    std::unique_ptr<Teacher> clone() const {
        return std::make_unique<Teacher>(education, character, subjects, price_min, price_high,
                                        allow_location, available_times);
    }
};

class UserManage {
private:
    std::unordered_map<std::string, std::unique_ptr<User>> users;
    std::unordered_map<std::string, std::unique_ptr<Teacher>> teachers;
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> relationships;
    std::vector<std::tuple<std::string, std::string, std::string, int, std::string>> records;

public:
    Teacher* FindTeacherRaw(const std::string& name) {
        auto it = teachers.find(name);
        return it != teachers.end() ? it->second.get() : nullptr;
    }

    void AddRelationship(const std::string& student_name, const std::string& teacher_name,
                     const std::string& subject, const std::string& student_time_slot,
                     std::string& selected_teacher_time_slot);

    UserManage(std::string user_file = user_file, std::string teacher_file = teacher_file) {
        LoadUsers(user_file);
        LoadTeachers(teacher_file);
        LoadRelationships(relationship_file);
        LoadRecords(record_file);
    }

    bool CompleteTeacherProfile(const std::string& name);
    bool Login(const std::string& type, const std::string& name, const std::string& passwd) const;
    bool Registered(const std::string& type, const std::string& name, const std::string& passwd);
    bool Delete(const std::string& name);
    std::unique_ptr<User>& FindUser(const std::string& name);
    std::unique_ptr<Teacher>& FindTeacher(const std::string& name);

    std::unique_ptr<User> fromFile(const std::string& data);
    void LoadUsers(std::string user_file);
    void SaveUsers();

    std::unique_ptr<Teacher> fromTeachFile(const std::string& data);
    void LoadTeachers(std::string teacher_file);
    void SaveTeachers();

    std::unordered_map<std::string, std::unique_ptr<Teacher>>& GetTeachers() {
        return teachers;
    }

    void AddRelationship(const std::string& student_name, const std::string& teacher_name,
                        const std::string& subject, const std::string& time_slot);
    void LoadRelationships(const std::string& file);
    void SaveRelationships();

    void AddRecord(const std::string& student_name, const std::string& teacher_name,
                   const std::string& subject, int score, const std::string& date);
    void LoadRecords(const std::string& file);
    void SaveRecords();

    std::vector<std::tuple<std::string, std::string, std::string, std::string>>& GetRelationships() {
        return relationships;
    }
    std::vector<std::tuple<std::string, std::string, std::string, int, std::string>>& GetRecords() {
        return records;
    }

    void updateTeacherTime(const std::string& teacher_name, const std::string& weekday,
                           int start, int end);
};

#endif