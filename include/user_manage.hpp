#ifndef USER_MANAGE_HPP
#define USER_MANAGE_HPP

#include <unordered_map>
#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <iostream>

class User {
public:
    User(const std::string& type, const std::string& name, const std::string& passwd);
    std::string toFileString();
    void changeName(const std::string& name) { user_name = name; }
    void changeType(const std::string& type) { user_type = type; }
    void changeHash(const std::string& hash) { hash_passwd = hash; }
    void changeSalt(const std::string& salt) { this->salt = salt; }
    std::string GetName() const { return user_name; }
    std::string GetType() const { return user_type; }
    std::string getHashPasswd() const { return hash_passwd; }
    std::string getSalt() const { return salt; }

private:
    std::string user_type;
    std::string user_name;
    std::string hash_passwd;
    std::string salt;
};

class Teacher {
public:
    explicit Teacher(const std::string& name);
    void add_available_time(const std::string& weekday, int start, int end);
    std::string ToTeachfilestring() const;
    std::string ToPendingfilestring() const;
    std::string GetName() const { return user_name; }
    void setName(const std::string& name) { user_name = name; }
    std::string education;
    std::string character;
    std::string id_number;
    std::vector<std::string> subjects;
    std::vector<std::string> allow_location;
    uint16_t price_min;
    uint16_t price_max;
    std::vector<std::pair<std::string, std::pair<int, int>>> available_times;
    std::vector<std::string> evaluations; // Used for self-description during registration

    const std::vector<std::pair<std::string, std::pair<int, int>>>& getAvailableTimes() const { return available_times; }

private:
    std::string user_name;
};

class UserManage {
public:
    static const std::string ADMIN_TYPE;
    UserManage(const std::string& user_file, const std::string& teacher_file,
               const std::string& relationship_file, const std::string& record_file,
               const std::string& pending_teacher_file);
    ~UserManage();

    bool Register(const std::string& user_type, const std::string& name, const std::string& password);
    bool RegisterTeacherPending(const std::string& name, const std::string& password, const std::string& id_number,
                                const std::string& education, const std::string& character,
                                const std::vector<std::string>& subjects, const std::vector<std::string>& locations,
                                uint16_t price_min, uint16_t price_max,
                                const std::vector<std::pair<std::string, std::pair<int, int>>>& available_times,
                                const std::string& self_description); // Added self-description
    bool ApproveTeacher(const std::string& name);
    bool RejectTeacher(const std::string& name);
    bool Login(const std::string& name, const std::string& password, std::string& user_type);
    bool Exist(const std::string& name) const;
    void LoadUsers();
    void SaveUsers();
    void LoadTeachers();
    void SaveTeachers();
    void LoadPendingTeachers();
    void SavePendingTeachers();
    void LoadRelationships();
    void SaveRelationships();
    void AddRelationship(const std::string& student_name, const std::string& teacher_name,
                         const std::string& subject, const std::string& time_slot);
    std::vector<std::tuple<std::string, std::string, std::string, std::string>>
        GetRelationshipsForUser(const std::string& username) const;
    void AddRecord(const std::string& student_name, const std::string& teacher_name,
                   const std::string& subject, int score, const std::string& date);
    void LoadRecords();
    void SaveRecords();
    bool Delete(const std::string& name);
    bool AddEvaluation(const std::string& teacher_name, const std::string& evaluation);
    std::vector<std::string> GetEvaluations(const std::string& teacher_name) const;

    const std::unordered_map<std::string, std::unique_ptr<User>>& GetUsers() const { return users; }
    const std::unordered_map<std::string, std::unique_ptr<Teacher>>& GetTeachers() const { return teachers; }
    const std::unordered_map<std::string, std::tuple<std::string, std::string, std::unique_ptr<Teacher>>>& GetPendingTeachers() const { return pending_teachers; }
    const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& GetRelationships() const { return relationships; }
    const std::vector<std::tuple<std::string, std::string, std::string, int, std::string>>& GetRecords() const { return records; }

private:
    std::unordered_map<std::string, std::unique_ptr<User>> users;
    std::unordered_map<std::string, std::unique_ptr<Teacher>> teachers;
    std::unordered_map<std::string, std::tuple<std::string, std::string, std::unique_ptr<Teacher>>> pending_teachers;
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> relationships;
    std::vector<std::tuple<std::string, std::string, std::string, int, std::string>> records;
    std::string user_file, teacher_file, relationship_file, record_file, pending_teacher_file;

    std::unique_ptr<Teacher> fromTeachFile(const std::string& data);
    std::unique_ptr<User> fromUserFile(const std::string& data);
    std::tuple<std::string, std::string, std::unique_ptr<Teacher>> fromPendingTeachFile(const std::string& data);
    void InitializeAdmin();
};

std::string hashPasswd(const std::string& password, const std::string& salt);
std::string generateSalt(size_t length);
std::string binaryToHex(const std::string& binary);
std::string hexToBinary(const std::string& hex);

#endif //USER_MANAGE_HPP