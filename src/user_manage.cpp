#include "user_manage.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <openssl/evp.h>
#include <QDebug>
#include <algorithm>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <random>
#include <QString>

const std::string UserManage::ADMIN_TYPE = "2";

std::string binaryToHex(const std::string& binary) {
    std::stringstream ss;
    for (unsigned char c : binary) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }
    return ss.str();
}

std::string hexToBinary(const std::string& hex) {
    std::string binary;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        char byte = static_cast<char>(std::stoul(byteString, nullptr, 16));
        binary += byte;
    }
    return binary;
}

std::string hashPasswd(const std::string& password, const std::string& salt) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        qDebug() << "无法创建 EVP_MD_CTX";
        return "";
    }
    const EVP_MD* md = EVP_sha256();
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;

    std::string saltedPassword = salt + password;

    if (EVP_DigestInit_ex(ctx, md, nullptr) != 1 ||
        EVP_DigestUpdate(ctx, saltedPassword.c_str(), saltedPassword.size()) != 1 ||
        EVP_DigestFinal_ex(ctx, digest, &digest_len) != 1) {
        qDebug() << "密码哈希失败";
        EVP_MD_CTX_free(ctx);
        return "";
    }
    EVP_MD_CTX_free(ctx);
    return binaryToHex(std::string(reinterpret_cast<char*>(digest), digest_len));
}

std::string generateSalt(size_t length) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 61);
    const std::string salt_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string salt;
    for (size_t i = 0; i < length; ++i) {
        salt += salt_chars[dis(gen)];
    }
    return salt;
}

User::User(const std::string& type, const std::string& name, const std::string& passwd)
    : user_type(type), user_name(name), hash_passwd("") {
    if (name.empty() || passwd.empty()) {
        qDebug() << "用户名或密码不能为空";
        return;
    }
    this->salt = generateSalt(16);
    this->hash_passwd = hashPasswd(passwd, this->salt);
}

std::string User::toFileString() {
    return user_type + "|" + user_name + "|" + hash_passwd + "|" + salt;
}

Teacher::Teacher(const std::string& name) : user_name(name), price_min(0), price_max(0) {}

void Teacher::add_available_time(const std::string& weekday, int start, int end) {
    available_times.push_back({weekday, {start, end} });
}

std::string Teacher::ToTeachfilestring() const {
    std::stringstream ss;
    std::string education_code = this->education;
    std::string character_code = this->character;
    if (education == "大学生家教") education_code = "0";
    else if (education == "专职家教") education_code = "1";
    else if (education == "在职教师") education_code = "2";
    if (character == "温和型") character_code = "0";
    else if (character == "严格型") character_code = "1";

    ss << user_name << "|"
       << education_code << "|"
       << character_code << "|";
    for (size_t i = 0; i < subjects.size(); ++i) {
        ss << subjects[i];
        if (i < subjects.size() - 1) ss << ",";
    }
    ss << "|"
       << price_min << "|"
       << price_max << "|";
    for (size_t i = 0; i < allow_location.size(); ++i) {
        ss << allow_location[i];
        if (i < allow_location.size() - 1) ss << ",";
    }
    ss << "|";
    for (size_t i = 0; i < available_times.size(); ++i) {
        ss << available_times[i].first << ","
           << available_times[i].second.first << ","
           << available_times[i].second.second;
        if (i < available_times.size() - 1) ss << "/";
    }
    ss << "|";
    for (size_t i = 0; i < evaluations.size(); ++i) {
        ss << evaluations[i];
        if (i < evaluations.size() - 1) ss << "/";
    }
    return ss.str();
}

std::string Teacher::ToPendingfilestring() const {
    std::stringstream ss;
    std::string education_code = this->education;
    std::string character_code = this->character;
    if (education == "大学生家教") education_code = "0";
    else if (education == "专职家教") education_code = "1";
    else if (education == "在职教师") education_code = "2";
    if (character == "温和型") character_code = "0";
    else if (character == "严格型") character_code = "1";

    ss << user_name << "|"
       << education_code << "|"
       << character_code << "|"
       << id_number << "|";
    for (size_t i = 0; i < subjects.size(); ++i) {
        ss << subjects[i];
        if (i < subjects.size() - 1) ss << ",";
    }
    ss << "|"
       << price_min << "|"
       << price_max << "|";
    for (size_t i = 0; i < allow_location.size(); ++i) {
        ss << allow_location[i];
        if (i < allow_location.size() - 1) ss << ",";
    }
    ss << "|";
    for (size_t i = 0; i < available_times.size(); ++i) {
        ss << available_times[i].first << ","
           << available_times[i].second.first << ","
           << available_times[i].second.second;
        if (i < available_times.size() - 1) ss << "/";
    }
    return ss.str();
}

UserManage::UserManage(const std::string &user_file, const std::string &teacher_file,
                       const std::string &relationship_file, const std::string &record_file,
                       const std::string &pending_teacher_file)
    : user_file(user_file), teacher_file(teacher_file),
      relationship_file(relationship_file), record_file(record_file),
      pending_teacher_file(pending_teacher_file)
{
    qDebug() << "用户文件:" << QString::fromStdString(user_file);
    qDebug() << "教师文件:" << QString::fromStdString(teacher_file);
    qDebug() << "待审教师文件:" << QString::fromStdString(pending_teacher_file);
    LoadUsers();
    InitializeAdmin();
    LoadTeachers();
    LoadPendingTeachers();
    LoadRelationships();
    LoadRecords();
    qDebug() << "UserManage 初始化完成";
}

UserManage::~UserManage() {
    qDebug() << "UserManage 析构函数: 保存";
    SaveUsers();
    SaveTeachers();
    SavePendingTeachers();
    SaveRelationships();
    SaveRecords();
    qDebug() << "UserManage 析构函数: 所有数据已保存";
}

void UserManage::InitializeAdmin() {
    if (!Exist("admin")) {
        qDebug() << "未找到 admin 用户，创建默认管理员账户";
        auto new_user = std::make_unique<User>(ADMIN_TYPE, "admin", "admin");
        users["admin"] = std::move(new_user);
        SaveUsers();
    } else {
        qDebug() << "admin 用户已存在";
    }
}

bool UserManage::Register(const std::string &user_type, const std::string &name, const std::string &password) {
    std::string cleaned_name = name;
    cleaned_name.erase(0, cleaned_name.find_first_not_of(" \t\r\n"));
    cleaned_name.erase(cleaned_name.find_last_not_of(" \t\r\n") + 1);
    qDebug() << "尝试注册用户:" << QString::fromStdString(cleaned_name) << " 原名:" << QString::fromStdString(name);

    if (Exist(cleaned_name)) {
        qDebug() << "注册失败：用户已存在" << QString::fromStdString(cleaned_name);
        return false;
    }

    if (user_type == "0") {
        auto newUser = std::make_unique<User>(user_type, cleaned_name, password);
        users[cleaned_name] = std::move(newUser);
        SaveUsers();
        qDebug() << "新用户注册成功：" << QString::fromStdString(cleaned_name) << " 类型：" << QString::fromStdString(user_type);
        return true;
    } else if (user_type == "1") {
        qDebug() << "教师注册需通过 RegisterTeacherPending 处理";
        return false;
    }
    qDebug() << "无效的用户类型：" << QString::fromStdString(user_type);
    return false;
}

bool UserManage::RegisterTeacherPending(const std::string &name, const std::string &password, const std::string &id_number,
                                        const std::string &education, const std::string &character,
                                        const std::vector<std::string> &subjects, const std::vector<std::string> &locations,
                                        uint16_t price_min, uint16_t price_max,
                                        const std::vector<std::pair<std::string, std::pair<int, int>>> &available_times,
                                        const std::string &self_description) {
    std::string cleaned_name = name;
    cleaned_name.erase(0, cleaned_name.find_first_not_of(" \t\r\n"));
    cleaned_name.erase(cleaned_name.find_last_not_of(" \t\r\n") + 1);

    if (Exist(cleaned_name) || pending_teachers.count(cleaned_name)) {
        qDebug() << "注册失败：用户或待审教师已存在：" << QString::fromStdString(cleaned_name);
        return false;
    }

    if (id_number.empty() || subjects.empty() || locations.empty() || password.empty() || available_times.empty() || self_description.empty()) {
        qDebug() << "注册失败：教师信息、密码或自我描述不完整";
        return false;
    }

    std::vector<std::string> valid_educations = {"大学生家教", "专职家教", "在职教师"};
    std::vector<std::string> valid_characters = {"温和型", "严格型"};
    if (std::find(valid_educations.begin(), valid_educations.end(), education) == valid_educations.end() ||
        std::find(valid_characters.begin(), valid_characters.end(), character) == valid_characters.end()) {
        qDebug() << "注册失败：无效的学历或性格类型，输入学历：" << QString::fromStdString(education)
                 << ", 输入性格：" << QString::fromStdString(character);
        return false;
    }

    auto new_teacher = std::make_unique<Teacher>(cleaned_name);
    new_teacher->id_number = id_number;
    new_teacher->education = education;
    new_teacher->character = character;
    new_teacher->subjects = subjects;
    new_teacher->allow_location = locations;
    new_teacher->price_min = price_min;
    new_teacher->price_max = price_max;
    for (const auto& time : available_times) {
        new_teacher->add_available_time(time.first, time.second.first, time.second.second);
    }
    new_teacher->evaluations.push_back(self_description);
    std::string salt = generateSalt(16);
    std::string hashed_password = hashPasswd(password, salt);
    pending_teachers[cleaned_name] = std::make_tuple(hashed_password, salt, std::move(new_teacher));
    SavePendingTeachers();
    qDebug() << "教师注册申请提交成功，等待审批：" << QString::fromStdString(cleaned_name);
    return true;
}

bool UserManage::ApproveTeacher(const std::string &name) {
    auto it = pending_teachers.find(name);
    if (it == pending_teachers.end()) {
        qDebug() << "审批失败：教师未找到" << QString::fromStdString(name);
        return false;
    }

    auto& [password, salt, teacher] = it->second;
    if (!teacher || teacher->subjects.empty() || teacher->allow_location.empty() || teacher->available_times.empty()) {
        qDebug() << "审批失败：教师信息不完整" << QString::fromStdString(name);
        return false;
    }

    auto new_user = std::make_unique<User>("1", name, "");
    new_user->changeHash(password);
    new_user->changeSalt(salt);
    users[name] = std::move(new_user);
    teacher->id_number.clear();
    teachers[name] = std::move(teacher);
    pending_teachers.erase(it);

    SaveUsers();
    SaveTeachers();
    SavePendingTeachers();
    qDebug() << "教师审批通过：" << QString::fromStdString(name);
    return true;
}

bool UserManage::RejectTeacher(const std::string &name) {
    auto it = pending_teachers.find(name);
    if (it == pending_teachers.end()) {
        qDebug() << "拒绝：教师未找到" << QString::fromStdString(name);
        return false;
    }
    pending_teachers.erase(it);
    SavePendingTeachers();
    qDebug() << "教师注册申请被拒绝：" << QString::fromStdString(name);
    return true;
}

bool UserManage::Login(const std::string &name, const std::string &password, std::string &user_type) {
    qDebug() << "尝试登录用户：" << QString::fromStdString(name);
    auto it = users.find(name);
    if (it == users.end()) {
        qDebug() << "登录失败：用户不存在：" << QString::fromStdString(name);
        return false;
    }

    User* user = it->second.get();
    std::string hashed_input_password = hashPasswd(password, user->getSalt());

    if (hashed_input_password == user->getHashPasswd()) {
        user_type = user->GetType();
        qDebug() << "登录成功！用户名：" << QString::fromStdString(name) << " 类型：" << QString::fromStdString(user_type);
        return true;
    } else {
        qDebug() << "登录失败：密码不正确：" << QString::fromStdString(name);
        return false;
    }
}

bool UserManage::Exist(const std::string &name) const {
    std::string cleaned_name = name;
    cleaned_name.erase(0, cleaned_name.find_first_not_of(" \t\r\n"));
    cleaned_name.erase(cleaned_name.find_last_not_of(" \t\r\n") + 1);
    bool exists = users.count(cleaned_name) > 0;
    qDebug() << "检查用户存在：" << QString::fromStdString(cleaned_name) << ", 结果：" << exists << ", 用户数：" << users.size();
    return exists;
}

bool UserManage::AddEvaluation(const std::string &teacher_name, const std::string &evaluation) {
    auto it = teachers.find(teacher_name);
    if (it == teachers.end()) {
        qDebug() << "教师未找到：" << QString::fromStdString(teacher_name);
        return false;
    }
    it->second->evaluations.push_back(evaluation);
    SaveTeachers();
    qDebug() << "评价添加成功，教师：" << QString::fromStdString(teacher_name);
    return true;
}

std::vector<std::string> UserManage::GetEvaluations(const std::string &teacher_name) const {
    auto it = teachers.find(teacher_name);
    if (it != teachers.end()) {
        return it->second->evaluations;
    }
    return {};
}

void UserManage::LoadUsers() {
    std::ifstream ifs(user_file);
    if (!ifs.is_open()) {
        qDebug() << "无法打开用户文件进行读取：" << QString::fromStdString(user_file);
        return;
    }
    users.clear();
    std::string line;
    while (std::getline(ifs, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (line.empty()) continue;

        std::unique_ptr<User> user = fromUserFile(line);
        if (user) {
            users[user->GetName()] = std::move(user);
        } else {
            qDebug() << "解析用户数据失败：" << QString::fromStdString(line);
        }
    }
    qDebug() << "从 " << QString::fromStdString(user_file) << " 加载了 " << users.size() << " 个用户";
    ifs.close();
}

void UserManage::SaveUsers() {
    std::ofstream ofs(user_file);
    if (!ofs.is_open()) {
        qDebug() << "无法打开用户文件进行写入：" << QString::fromStdString(user_file);
        return;
    }
    for (const auto &pair : users) {
        ofs << pair.second->toFileString() << "\n";
    }
    ofs.close();
    qDebug() << "保存了 " << users.size() << " 个用户到 " << QString::fromStdString(user_file);
}

void UserManage::LoadTeachers() {
    std::ifstream ifs(teacher_file);
    if (!ifs.is_open()) {
        qDebug() << "无法打开教师文件进行读取：" << QString::fromStdString(teacher_file);
        return;
    }
    teachers.clear();
    std::string line;
    while (std::getline(ifs, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (line.empty()) continue;

        std::unique_ptr<Teacher> teacher = fromTeachFile(line);
        if (teacher) {
            teachers[teacher->GetName()] = std::move(teacher);
        } else {
            qDebug() << "解析教师数据失败：" << QString::fromStdString(line);
        }
    }
    qDebug() << "从 " << QString::fromStdString(teacher_file) << " 加载了 " << teachers.size() << " 个教师";
    ifs.close();
}

void UserManage::SaveTeachers() {
    std::map<std::string, std::string> existing_teachers;
    std::ifstream ifs(teacher_file);
    if (ifs.is_open()) {
        std::string line;
        while (std::getline(ifs, line)) {
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            if (line.empty()) continue;
            size_t pos = line.find('|');
            if (pos != std::string::npos) {
                std::string name = line.substr(0, pos);
                existing_teachers[name] = line;
            }
        }
        ifs.close();
    }

    for (const auto &pair : teachers) {
        existing_teachers[pair.first] = pair.second->ToTeachfilestring();
    }

    std::ofstream ofs(teacher_file);
    if (!ofs.is_open()) {
        qDebug() << "无法打开教师文件进行写入：" << QString::fromStdString(teacher_file);
        return;
    }
    for (const auto &pair : existing_teachers) {
        ofs << pair.second << "\n";
    }
    qDebug() << "保存了 " << existing_teachers.size() << " 个教师到 " << QString::fromStdString(teacher_file);
    ofs.close();
}

void UserManage::LoadPendingTeachers() {
    std::ifstream ifs(pending_teacher_file);
    if (!ifs.is_open()) {
        qDebug() << "无法打开待审教师文件进行读取：" << QString::fromStdString(pending_teacher_file);
        return;
    }
    pending_teachers.clear();
    std::string line;
    int line_number = 0;
    while (std::getline(ifs, line)) {
        ++line_number;
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (line.empty()) {
            qDebug() << "跳过空行，行号：" << line_number;
            continue;
        }

        auto [password, salt, teacher] = fromPendingTeachFile(line);
        if (teacher && !password.empty()) {
            qDebug() << "成功加载待审教师：" << QString::fromStdString(teacher->GetName()) << ", 行号：" << line_number;
            pending_teachers[teacher->GetName()] = std::make_tuple(password, salt, std::move(teacher));
        } else {
            qDebug() << "解析待审教师数据失败，行号：" << line_number << ", 数据：" << QString::fromStdString(line);
        }
    }
    qDebug() << "从 " << QString::fromStdString(pending_teacher_file) << " 加载了 " << pending_teachers.size() << " 个待审教师";
    ifs.close();
}

void UserManage::SavePendingTeachers() {
    std::ofstream ofs(pending_teacher_file);
    if (!ofs.is_open()) {
        qDebug() << "无法打开待审教师文件进行写入：" << QString::fromStdString(pending_teacher_file);
        return;
    }
    for (const auto &pair : pending_teachers) {
        const auto &[password, salt, teacher] = pair.second;
        ofs << pair.first << "|" << password << "|" << salt << "|"
            << teacher->education << "|"
            << teacher->character << "|"
            << teacher->id_number << "|";
        for (size_t i = 0; i < teacher->subjects.size(); ++i) {
            ofs << teacher->subjects[i];
            if (i < teacher->subjects.size() - 1) ofs << ",";
        }
        ofs << "|" << teacher->price_min << "|"
            << teacher->price_max << "|";
        for (size_t i = 0; i < teacher->allow_location.size(); ++i) {
            ofs << teacher->allow_location[i];
            if (i < teacher->allow_location.size() - 1) ofs << ",";
        }
        ofs << "|";
        for (size_t i = 0; i < teacher->available_times.size(); ++i) {
            ofs << teacher->available_times[i].first << ","
                << teacher->available_times[i].second.first << ","
                << teacher->available_times[i].second.second;
            if (i < teacher->available_times.size() - 1) ofs << "/";
        }
        ofs << "|";
        for (size_t i = 0; i < teacher->evaluations.size(); ++i) {
            ofs << teacher->evaluations[i];
            if (i < teacher->evaluations.size() - 1) ofs << "/";
        }
        ofs << "\n";
    }
    qDebug() << "保存了 " << pending_teachers.size() << " 个待审教师到 " << QString::fromStdString(pending_teacher_file);
    ofs.close();
}

void UserManage::LoadRelationships() {
    std::ifstream ifs(relationship_file);
    if (!ifs.is_open()) {
        qDebug() << "无法打开关系文件进行读取：" << QString::fromStdString(relationship_file);
        return;
    }
    relationships.clear();
    std::string line;
    while (std::getline(ifs, line)) {
        std::stringstream ss(line);
        std::string student_name, teacher_name, subject, time_slot;
        if (std::getline(ss, student_name, '|') &&
            std::getline(ss, teacher_name, '|') &&
            std::getline(ss, subject, '|') &&
            std::getline(ss, time_slot)) {
            relationships.emplace_back(student_name, teacher_name, subject, time_slot);
        } else {
            qDebug() << "解析关系数据失败：" << QString::fromStdString(line);
        }
    }
    qDebug() << "从 " << QString::fromStdString(relationship_file) << " 加载了 " << relationships.size() << " 条关系";
    ifs.close();
}

void UserManage::SaveRelationships() {
    std::ofstream ofs(relationship_file);
    if (!ofs.is_open()) {
        qDebug() << "无法打开关系文件进行写入：" << QString::fromStdString(relationship_file);
        return;
    }
    for (const auto &rel : relationships) {
        ofs << std::get<0>(rel) << "|" << std::get<1>(rel) << "|" << std::get<2>(rel) << "|" << std::get<3>(rel) << "\n";
    }
    qDebug() << "保存了 " << relationships.size() << " 条关系到 " << QString::fromStdString(relationship_file);
    ofs.close();
}

void UserManage::AddRelationship(const std::string &student_name, const std::string &teacher_name,
                                 const std::string &subject, const std::string &time_slot) {
    for (const auto &rel : relationships) {
        if (std::get<0>(rel) == student_name &&
            std::get<1>(rel) == teacher_name &&
            std::get<2>(rel) == subject &&
            std::get<3>(rel) == time_slot) {
            qDebug() << "关系已存在：学生=" << QString::fromStdString(student_name)
                     << ", 教师=" << QString::fromStdString(teacher_name)
                     << ", 科目=" << QString::fromStdString(subject)
                     << ", 时间段=" << QString::fromStdString(time_slot);
            return;
        }
    }
    relationships.emplace_back(student_name, teacher_name, subject, time_slot);
    qDebug() << "添加关系：学生=" << QString::fromStdString(student_name)
             << ", 教师=" << QString::fromStdString(teacher_name)
             << ", 科目=" << QString::fromStdString(subject)
             << ", 时间段=" << QString::fromStdString(time_slot);
    SaveRelationships();
}

std::vector<std::tuple<std::string, std::string, std::string, std::string>>
UserManage::GetRelationshipsForUser(const std::string &user_name) const {
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> user_rels;
    for (const auto &rel : relationships) {
        if (std::get<0>(rel) == user_name || std::get<1>(rel) == user_name) {
            user_rels.push_back(rel);
        }
    }
    return user_rels;
}

void UserManage::AddRecord(const std::string &student_name, const std::string &teacher_name,
                           const std::string &subject, int score, const std::string &date) {
    records.emplace_back(student_name, teacher_name, subject, score, date);
    qDebug() << "添加记录：学生=" << QString::fromStdString(student_name)
             << ", 教师=" << QString::fromStdString(teacher_name)
             << ", 科目=" << QString::fromStdString(subject)
             << ", 分数=" << score
             << ", 日期=" << QString::fromStdString(date);
    SaveRecords();
}

void UserManage::LoadRecords() {
    std::ifstream ifs(record_file);
    if (!ifs.is_open()) {
        qDebug() << "无法打开记录文件进行读取：" << QString::fromStdString(record_file);
        return;
    }
    records.clear();
    std::string line;
    while (std::getline(ifs, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string student_name, teacher_name, subject, score_str, date;
        if (std::getline(ss, student_name, '|') &&
            std::getline(ss, teacher_name, '|') &&
            std::getline(ss, subject, '|') &&
            std::getline(ss, score_str, '|') &&
            std::getline(ss, date)) {
            try {
                int score = std::stoi(score_str);
                records.emplace_back(student_name, teacher_name, subject, score, date);
            } catch (const std::exception &e) {
                qDebug() << "解析记录分数失败：" << QString::fromStdString(line) << ", 错误：" << e.what();
            }
        } else {
            qDebug() << "解析记录数据失败：" << QString::fromStdString(line);
        }
    }
    qDebug() << "从 " << QString::fromStdString(record_file) << " 加载了 " << records.size() << " 条记录";
    ifs.close();
}

void UserManage::SaveRecords() {
    std::ofstream ofs(record_file);
    if (!ofs.is_open()) {
        qDebug() << "无法打开记录文件进行写入：" << QString::fromStdString(record_file);
        return;
    }
    for (const auto &rec : records) {
        ofs << std::get<0>(rec) << "|" << std::get<1>(rec) << "|" << std::get<2>(rec) << "|"
            << std::get<3>(rec) << "|" << std::get<4>(rec) << "\n";
    }
    qDebug() << "保存了 " << records.size() << " 条记录到 " << QString::fromStdString(record_file);
    ofs.close();
}

bool UserManage::Delete(const std::string &name) {
    auto user_it = users.find(name);
    if (user_it == users.end()) {
        qDebug() << "无法删除：用户不存在：" << QString::fromStdString(name);
        return false;
    }

    users.erase(user_it);

    auto teacher_it = teachers.find(name);
    if (teacher_it != teachers.end()) {
        teachers.erase(teacher_it);
    }

    auto pending_it = pending_teachers.find(name);
    if (pending_it != pending_teachers.end()) {
        pending_teachers.erase(pending_it);
    }

    relationships.erase(
        std::remove_if(
            relationships.begin(),
            relationships.end(),
            [&name](const auto &it) {
                return std::get<0>(it) == name || std::get<1>(it) == name;
            }),
        relationships.end());

    records.erase(
        std::remove_if(
            records.begin(),
            records.end(),
            [&name](const auto &it) {
                return std::get<0>(it) == name || std::get<1>(it) == name;
            }),
        records.end());

    SaveUsers();
    SaveTeachers();
    SavePendingTeachers();
    SaveRelationships();
    SaveRecords();
    qDebug() << "用户已删除成功：" << QString::fromStdString(name);
    return true;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        token.erase(0, token.find_first_not_of(" \t\r\n"));
        token.erase(token.find_last_not_of(" \t\r\n") + 1);
        if (!token.empty()) tokens.push_back(token);
    }
    return tokens;
}

std::unique_ptr<Teacher> UserManage::fromTeachFile(const std::string &data) {
    std::vector<std::string> fields = split(data, '|');
    if (fields.size() < 8) {
        qDebug() << "教师数据字段不足：" << QString::fromStdString(data) << ", 找到 " << fields.size() << " fields";
        return nullptr;
    }

    std::string name = fields[0];
    std::string education_code = fields[1];
    std::string character_code = fields[2];
    std::string subjects_str = fields[3];
    std::string price_min_str = fields[4];
    std::string price_max_str = fields[5];
    std::string locations_str = fields[6];
    std::string times_str = fields[7];
    std::string evaluations_str = fields.size() > 8 ? fields[8] : "";

    name.erase(std::remove(name.begin(), name.end(), '"'), name.end());

    qDebug() << "解析教师数据：姓名='" << QString::fromStdString(name)
             << "', 学历代码=" << QString::fromStdString(education_code)
             << ", 性格代码=" << QString::fromStdString(character_code);

    std::string education;
    if (education_code == "0") education = "大学生家教";
    else if (education_code == "1") education = "专职家教";
    else if (education_code == "2") education = "在职教师";
    else {
        qDebug() << "无效的学历代码：" << QString::fromStdString(education_code);
        return nullptr;
    }

    std::string character;
    if (character_code == "0") character = "温和型";
    else if (character_code == "1") character = "严格型";
    else {
        qDebug() << "无效的性格代码：" << QString::fromStdString(character_code);
        return nullptr;
    }

    auto teacher = std::make_unique<Teacher>(name);
    teacher->education = education;
    teacher->character = character;

    std::vector<std::string> subjects = split(subjects_str, ',');
    for (const auto& subject : subjects) {
        if (!subject.empty()) {
            std::string normalized_subject = subject;
            if (normalized_subject == "physical" || normalized_subject == "Physical") normalized_subject = "物理";
            if (normalized_subject == "math" || normalized_subject == "Math") normalized_subject = "数学";
            teacher->subjects.push_back(normalized_subject);
            qDebug() << "添加科目：" << QString::fromStdString(normalized_subject);
        }
    }
    if (teacher->subjects.empty()) {
        qDebug() << "教师无有效科目：" << QString::fromStdString(name);
        return nullptr;
    }

    try {
        if (!std::all_of(price_min_str.begin(), price_min_str.end(), ::isdigit) ||
            !std::all_of(price_max_str.begin(), price_max_str.end(), ::isdigit)) {
            qDebug() << "价格范围非数字：" << QString::fromStdString(price_min_str)
                     << ", " << QString::fromStdString(price_max_str);
            return nullptr;
        }
        uint16_t price_min = std::stoi(price_min_str);
        uint16_t price_max = std::stoi(price_max_str);
        if (price_min <= 0 || price_min > price_max) {
            qDebug() << "无效的价格范围：" << price_min << " - " << price_max;
            return nullptr;
        }
        teacher->price_min = price_min;
        teacher->price_max = price_max;
        qDebug() << "价格范围：" << price_min << "-" << price_max;
    } catch (const std::exception &e) {
        qDebug() << "解析教师价格失败：" << QString::fromStdString(price_min_str)
                 << ", " << QString::fromStdString(price_max_str)
                 << ", 错误：" << e.what();
        return nullptr;
    }

    std::vector<std::string> locations = split(locations_str, ',');
    for (const auto& loc : locations) {
        if (!loc.empty()) {
            teacher->allow_location.push_back(loc);
            qDebug() << "添加地点：" << QString::fromStdString(loc);
        }
    }
    if (teacher->allow_location.empty()) {
        qDebug() << "教师无有效地点：" << QString::fromStdString(name);
        return nullptr;
    }

    std::vector<std::string> time_slots = split(times_str, '/');
    for (const auto& time_slot : time_slots) {
        std::vector<std::string> time_parts = split(time_slot, ',');
        if (time_parts.size() == 3) {
            try {
                std::string weekday = time_parts[0];
                int start = std::stoi(time_parts[1]);
                int end = std::stoi(time_parts[2]);
                if (start < end && start >= 0 && end <= 2400) {
                    teacher->add_available_time(weekday, start, end);
                    qDebug() << "添加时间段：" << QString::fromStdString(weekday)
                             << ", " << start << "-" << end;
                } else {
                    qDebug() << "无效时间段：" << QString::fromStdString(time_slot) << ", start >= end 或超出范围";
                }
            } catch (const std::exception &e) {
                qDebug() << "解析时间段失败：" << QString::fromStdString(time_slot)
                         << ", 错误：" << e.what();
            }
        } else {
            qDebug() << "时间段格式错误：" << QString::fromStdString(time_slot);
        }
    }
    if (teacher->available_times.empty()) {
        qDebug() << "教师无有效时间段：" << QString::fromStdString(name);
        return nullptr;
    }

    if (!evaluations_str.empty()) {
        std::vector<std::string> evaluations = split(evaluations_str, '/');
        for (const auto& eval : evaluations) {
            if (!eval.empty()) {
                teacher->evaluations.push_back(eval);
                qDebug() << "添加评价：" << QString::fromStdString(eval);
            }
        }
    }

    return teacher;
}

std::tuple<std::string, std::string, std::unique_ptr<Teacher>> UserManage::fromPendingTeachFile(const std::string& data) {
    std::vector<std::string> fields = split(data, '|');
    if (fields.size() < 11) {
        qDebug() << "无效的待审教师数据：字段不足，找到 " << fields.size();
        return {"", "", nullptr};
    }

    try {
        std::string name = fields[0];
        std::string password = fields[1];
        std::string salt = fields[2];
        std::string education = fields[3];
        std::string character = fields[4];
        std::string id_number = fields[5];
        std::string subjects_str = fields[6];
        std::string price_min_str = fields[7];
        std::string price_max_str = fields[8];
        std::string locations_str = fields[9];
        std::string times_str = fields[10];
        std::string evaluations_str = fields.size() > 11 ? fields[11] : "";

        name.erase(std::remove(name.begin(), name.end(), '"'), name.end());

        qDebug() << "解析待审教师：姓名='" << QString::fromStdString(name)
                 << "', 学历=" << QString::fromStdString(education)
                 << ", 性格=" << QString::fromStdString(character)
                 << ", ID=" << QString::fromStdString(id_number);

        std::string education_name;
        if (education == "0") education_name = "大学生家教";
        else if (education == "1") education_name = "专职家教";
        else if (education == "2") education_name = "在职教师";
        else {
            qDebug() << "无效的学历代码：" << QString::fromStdString(education);
            return {"", "", nullptr};
        }

        std::string character_name;
        if (character == "0") character_name = "温和型";
        else if (character == "1") character_name = "严格型";
        else {
            qDebug() << "无效的性格代码：" << QString::fromStdString(character);
            return {"", "", nullptr};
        }

        auto teacher = std::make_unique<Teacher>(name);
        teacher->education = education_name;
        teacher->character = character_name;
        teacher->id_number = id_number;

        std::vector<std::string> subjects = split(subjects_str, ',');
        for (const auto& subject : subjects) {
            if (!subject.empty()) {
                teacher->subjects.push_back(subject);
            }
        }
        if (teacher->subjects.empty()) {
            qDebug() << "待审教师无有效科目：" << QString::fromStdString(name);
            return {"", "", nullptr};
        }

        if (!std::all_of(price_min_str.begin(), price_min_str.end(), ::isdigit) ||
            !std::all_of(price_max_str.begin(), price_max_str.end(), ::isdigit)) {
            qDebug() << "价格范围非数字：" << QString::fromStdString(price_min_str)
                     << ", " << QString::fromStdString(price_max_str);
            return {"", "", nullptr};
        }
        uint16_t price_min = std::stoi(price_min_str);
        uint16_t price_max = std::stoi(price_max_str);
        if (price_min <= 0 || price_min > price_max) {
            qDebug() << "无效的价格范围：" << price_min << "-" << price_max;
            return {"", "", nullptr};
        }
        teacher->price_min = price_min;
        teacher->price_max = price_max;

        std::vector<std::string> locations = split(locations_str, ',');
        for (const auto& loc : locations) {
            if (!loc.empty()) {
                teacher->allow_location.push_back(loc);
            }
        }
        if (teacher->allow_location.empty()) {
            qDebug() << "待审教师无有效地点：" << QString::fromStdString(name);
            return {"", "", nullptr};
        }

        std::vector<std::string> time_slots = split(times_str, '/');
        for (const auto& time_slot : time_slots) {
            std::vector<std::string> time_parts = split(time_slot, ',');
            if (time_parts.size() == 3) {
                std::string weekday = time_parts[0];
                int start_time = std::stoi(time_parts[1]);
                int end_time = std::stoi(time_parts[2]);
                if (start_time < end_time && start_time >= 0 && end_time <= 2400) {
                    teacher->add_available_time(weekday, start_time, end_time);
                } else {
                    qDebug() << "无效时间段：" << QString::fromStdString(time_slot);
                }
            } else {
                qDebug() << "无效时间段格式：" << QString::fromStdString(time_slot);
            }
        }
        if (teacher->available_times.empty()) {
            qDebug() << "待审教师无有效时间段：" << QString::fromStdString(name);
            return {"", "", nullptr};
        }

        if (!evaluations_str.empty()) {
            std::vector<std::string> evaluations = split(evaluations_str, '/');
            for (const auto& eval : evaluations) {
                if (!eval.empty()) {
                    teacher->evaluations.push_back(eval);
                }
            }
        }

        return {password, salt, std::move(teacher)};
    } catch (const std::exception& e) {
        qDebug() << "解析待审教师数据出错：" << QString::fromStdString(data) << ", 错误：" << e.what();
        return {"", "", nullptr};
    }
}

std::unique_ptr<User> UserManage::fromUserFile(const std::string &data) {
    std::stringstream ss(data);
    std::string type, name, hash, salt;
    if (std::getline(ss, type, '|') &&
        std::getline(ss, name, '|') &&
        std::getline(ss, hash, '|') &&
        std::getline(ss, salt)) {
        auto user = std::make_unique<User>("", "", "");
        user->changeType(type);
        user->changeName(name);
        user->changeHash(hash);
        user->changeSalt(salt);
        qDebug() << "用户解析成功：姓名=" << QString::fromStdString(name)
                 << ", 类型=" << QString::fromStdString(type);
        return user;
    }
    qDebug() << "解析用户文件行失败：" << QString::fromStdString(data);
    return nullptr;
}