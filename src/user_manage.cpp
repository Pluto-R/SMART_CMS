#include "user_manage.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <openssl/evp.h>
#include <QDebug>
#include <QTextStream> // Added for Qt::endl

std::string hashPasswd(const std::string& password, const std::string& salt) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sha256();
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;

    EVP_DigestInit_ex(ctx, md, nullptr);
    EVP_DigestUpdate(ctx, (salt + password).c_str(), (salt + password).size());
    EVP_DigestFinal_ex(ctx, digest, &digest_len);
    EVP_MD_CTX_free(ctx);

    return binaryToHex(std::string(reinterpret_cast<char*>(digest), digest_len));
}

std::string generateSalt(size_t length) {
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string salt;
    salt.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        salt += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return salt;
}

std::string binaryToHex(const std::string& binary) {
    static const char hexDigits[] = "0123456789abcdef";
    std::string hex;
    for (unsigned char c : binary) {
        hex += hexDigits[c >> 4];
        hex += hexDigits[c & 0xF];
    }
    return hex;
}

bool UserManage::Registered(const std::string& type, const std::string& name, const std::string& passwd) {
    if (users.find(name) != users.end()) {
        std::cout << "Failed to register! User already exists." << Qt::endl;
        return false;
    }

    users.emplace(name, std::make_unique<User>(type, name, passwd));
    std::cout << "Success register!" << Qt::endl;
    SaveUsers();

    if (type == "1") {
        teachers.emplace(name, std::make_unique<Teacher>(name));
        SaveTeachers();
    }
    return true;
}

bool UserManage::CompleteTeacherProfile(const std::string& name) {
    auto it = teachers.find(name);
    if (it == teachers.end()) {
        std::cout << "Teacher not found: " << name << Qt::endl;
        return false;
    }

    Teacher& teacher = *it->second;

    std::string education, character, subjects_str, locations_str, times_str;
    uint16_t price_min, price_high;
    std::vector<std::string> subjects, locations;
    std::vector<std::pair<std::string, std::pair<int, int>>> available_times;

    std::cout << "请输入老师的学历: 0:大学生家教 1:在职教师 2:特级教师: ";
    std::getline(std::cin, education);

    std::cout << "请输入您的性格：0：亲和型 1：权威型: ";
    std::getline(std::cin, character);

    std::cout << "输入您教学科目（以逗号分隔）：";
    std::getline(std::cin, subjects_str);
    std::istringstream subjects_iss(subjects_str);
    std::string subject;
    while (std::getline(subjects_iss, subject, ',')) {
        if (!subject.empty()) {
            subjects.push_back(subject);
        }
    }

    std::cout << "输入最低价格: ";
    std::cin >> price_min;

    std::cout << "输入最高价格: ";
    std::cin >> price_high;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "输入可教学的地点（以逗号分隔）: ";
    std::getline(std::cin, locations_str);
    std::istringstream locations_iss(locations_str);
    std::string location;
    while (std::getline(locations_iss, location, ',')) {
        if (!location.empty()) {
            locations.push_back(location);
        }
    }

    std::cout << "输入可用时间（格式: Tue,1200,1400/Wed,0900,1100）: ";
    std::getline(std::cin, times_str);
    std::istringstream times_iss(times_str);
    std::string time_entry;
    while (std::getline(times_iss, time_entry, '/')) {
        std::istringstream entry_ss(time_entry);
        std::string day, start_str, end_str;
        if (std::getline(entry_ss, day, ',') &&
            std::getline(entry_ss, start_str, ',') &&
            std::getline(entry_ss, end_str)) {
            try {
                int start = std::stoi(start_str);
                int end = std::stoi(end_str);
                available_times.emplace_back(day, std::make_pair(start, end));
            } catch (const std::exception& e) {
                std::cout << "Invalid time format: " << time_entry << ", skipping" << Qt::endl;
            }
        }
    }

    teacher.education = education;
    teacher.character = character;
    teacher.subjects = subjects;
    teacher.price_min = price_min;
    teacher.price_high = price_high;
    teacher.allow_location = locations;
    teacher.available_times = available_times;

    SaveTeachers();
    return true;
}

std::unique_ptr<User>& UserManage::FindUser(const std::string& name) {
    auto it = users.find(name);
    if (it != users.end() && it->second) {
        return it->second;
    }
    throw std::runtime_error("User not found: " + name);
}

std::unique_ptr<Teacher>& UserManage::FindTeacher(const std::string& name) {
    auto it = teachers.find(name);
    if (it != teachers.end()) {
        return it->second;
    }
    throw std::runtime_error("Teacher not found: " + name);
}

bool UserManage::Login(const std::string& type, const std::string& name, const std::string& passwd) const {
    auto it = users.find(name);
    if (it != users.end()) {
        bool success_passwd = (it->second->Gethash_passwd() == binaryToHex(hashPasswd(passwd, it->second->Getsalt())));
        bool success_type = (it->second->GetType() == type);
        if (success_passwd && success_type) {
            std::cout << "Success to login!" << Qt::endl;
            return true;
        } else {
            if (!success_type) std::cout << "The type is error!" << Qt::endl;
            if (!success_passwd) std::cout << "The passwd is error!" << Qt::endl;
            return false;
        }
    }
    std::cout << "Failed to login! Not exist user!" << Qt::endl;
    return false;
}

void UserManage::LoadUsers(std::string user_file) {
    std::ifstream file(user_file);
    if (!file) {
        std::cerr << "Failed to open users file: " << user_file << Qt::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (auto user = fromFile(line)) {
            users.insert({user->GetName(), std::move(user)});
        }
    }
    file.close();
}

void UserManage::LoadTeachers(std::string teacher_file) {
    std::ifstream file(teacher_file);
    if (!file) {
        std::cerr << "Failed to open teachers file: " << teacher_file << Qt::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (auto teacher = fromTeachFile(line)) {
            if (teachers.find(teacher->GetName()) == teachers.end()) {
                teachers.insert({teacher->GetName(), std::move(teacher)});
            }
        }
    }
    file.close();
}

void UserManage::SaveUsers() {
    std::ofstream file(user_file);
    if (!file) {
        std::cerr << "Failed to open users file for writing: " << user_file << Qt::endl;
        return;
    }
    for (const auto& pair : users) {
        file << pair.second->Tofilestring() << "\n";
    }
    std::cout << "Saved " << users.size() << " users to " << user_file << Qt::endl;
    file.close();
}

void UserManage::SaveTeachers() {
    std::ofstream file(teacher_file);
    if (!file) {
        std::cerr << "Failed to open teachers file for writing: " << teacher_file << Qt::endl;
        return;
    }
    for (const auto& pair : teachers) {
        file << pair.second->ToTeachfilestring() << "\n";
    }
    std::cout << "Saved " << teachers.size() << " teachers to " << teacher_file << Qt::endl;
    file.close();
}

std::string User::Tofilestring() {
    return user_name + ',' + user_type + ',' + hash_passwd + ',' + salt;
}

std::string Teacher::ToTeachfilestring() {
    std::string course;
    for (auto it = subjects.begin(); it != subjects.end(); ++it) {
        course += *it;
        if (it != subjects.end() - 1) course += ",";
    }
    std::string locations;
    for (auto it = allow_location.begin(); it != allow_location.end(); ++it) {
        locations += *it;
        if (it != allow_location.end() - 1) locations += ",";
    }
    std::string times;
    for (auto it = available_times.begin(); it != available_times.end(); ++it) {
        times += it->first + "," + std::to_string(it->second.first) + "," + std::to_string(it->second.second);
        if (std::next(it) != available_times.end()) times += "/";
    }
    std::string price_m = std::to_string(price_min);
    std::string price_h = std::to_string(price_high);
    return user_name + "|" + education + "|" + character + "|" + course + "|" + price_m + "|" +
           price_h + "|" + locations + "|" + times;
}

std::unique_ptr<User> UserManage::fromFile(const std::string& data) {
    std::istringstream iss(data);
    std::string name, type, hash, salt;
    if (std::getline(iss, name, ',') &&
        std::getline(iss, type, ',') &&
        std::getline(iss, hash, ',') &&
        std::getline(iss, salt)) {
        std::unique_ptr<User> ptr = std::make_unique<User>(type, name, "");
        ptr->ChangeName(name);
        ptr->ChangeType(type);
        ptr->ChangeHash(hash);
        ptr->ChangeSalt(salt);
        return ptr;
    }
    return nullptr;
}

std::unique_ptr<Teacher> UserManage::fromTeachFile(const std::string& data) {
    std::istringstream iss(data);
    std::string name, education, character, subjects, price_m, price_h, locations, times;

    std::unique_ptr<Teacher> teacher = std::make_unique<Teacher>();
    if (!std::getline(iss, name, '|') ||
        !std::getline(iss, education, '|') ||
        !std::getline(iss, character, '|') ||
        !std::getline(iss, subjects, '|') ||
        !std::getline(iss, price_m, '|') ||
        !std::getline(iss, price_h, '|') ||
        !std::getline(iss, locations, '|') ||
        !std::getline(iss, times)) {
        return nullptr;
    }

    teacher->ChangeName(name);
    teacher->education = education;
    teacher->character = character;

    std::istringstream course_ss(subjects);
    std::string subject;
    while (std::getline(course_ss, subject, ',')) {
        if (!subject.empty()) {
            teacher->subjects.push_back(subject);
        }
    }

    try {
        teacher->price_min = std::stoi(price_m);
        teacher->price_high = std::stoi(price_h);
    } catch (const std::exception& e) {
        return nullptr;
    }

    std::istringstream locations_ss(locations);
    std::string location;
    while (std::getline(locations_ss, location, ',')) {
        if (!location.empty()) {
            teacher->allow_location.push_back(location);
        }
    }

    std::istringstream times_ss(times);
    std::string time_entry;
    while (std::getline(times_ss, time_entry, '/')) {
        std::istringstream entry_ss(time_entry);
        std::string day, start_str, end_str;
        if (std::getline(entry_ss, day, ',') &&
            std::getline(entry_ss, start_str, ',') &&
            std::getline(entry_ss, end_str)) {
            try {
                int start = std::stoi(start_str);
                int end = std::stoi(end_str);
                teacher->available_times.emplace_back(day, std::make_pair(start, end));
            } catch (const std::exception& e) {
                continue;
            }
        }
    }

    return teacher;
}

void UserManage::AddRelationship(const std::string& student_name, const std::string& teacher_name,
                                const std::string& subject, const std::string& time_slot) {
    std::stringstream ss(time_slot);
    std::string day, start_str, end_str;
    if (!std::getline(ss, day, ',') || !std::getline(ss, start_str, ',') || !std::getline(ss, end_str)) {
        qDebug() << "[UserManage] Invalid time slot format:" << QString::fromStdString(time_slot);
        throw std::runtime_error("Invalid time slot format: " + time_slot);
    }

    int start, end;
    try {
        start = std::stoi(start_str);
        end = std::stoi(end_str);
    } catch (const std::exception& e) {
        qDebug() << "[UserManage] Invalid time values:" << QString::fromStdString(time_slot);
        throw std::runtime_error("Invalid time values: " + time_slot);
    }

    if (start >= end || start < 0 || end > 2359) {
        qDebug() << "[UserManage] Invalid time range:" << start << "-" << end;
        throw std::runtime_error("Invalid time range: " + std::to_string(start) + "-" + std::to_string(end));
    }

    auto& teacher = FindTeacher(teacher_name);

    bool time_available = false;
    for (const auto& t : teacher->available_times) {
        if (t.first == day && t.second.first == start && t.second.second == end) {
            time_available = true;
            break;
        }
    }
    if (!time_available) {
        qDebug() << "[UserManage] Teacher" << QString::fromStdString(teacher_name)
                 << " has no exact time slot:" << QString::fromStdString(time_slot);
        throw std::runtime_error("Teacher does not have exact time slot: " + time_slot);
    }

    for (const auto& rel : relationships) {
        if (std::get<1>(rel) == teacher_name) {
            std::stringstream rel_ss(std::get<3>(rel));
            std::string rel_day, rel_start_str, rel_end_str;
            if (std::getline(rel_ss, rel_day, ',') &&
                std::getline(rel_ss, rel_start_str, ',') &&
                std::getline(rel_ss, rel_end_str)) {
                try {
                    int rel_start = std::stoi(rel_start_str);
                    int rel_end = std::stoi(rel_end_str);
                    if (rel_day == day && start < rel_end && end > rel_start) {
                        qDebug() << "[UserManage] time slot " << QString::fromStdString(time_slot)
                                 << " conflicts with existing time slot for teacher "
                                 << QString::fromStdString(teacher_name) << ":"
                                 << QString::fromStdString(std::get<3>(rel));
                        throw std::runtime_error("Time slot conflicts with existing time slot");
                    }
                } catch (const std::exception& e) {
                    qDebug() << "[UserManage] Invalid time slot format in relationship:"
                             << QString::fromStdString(std::get<3>(rel));
                }
            }
        }
    }

    relationships.emplace_back(student_name, teacher_name, subject, time_slot);
    qDebug() << "[UserManage] Added relationship: student=" << QString::fromStdString(student_name)
             << ", teacher=" << QString::fromStdString(teacher_name)
             << ", subject=" << QString::fromStdString(subject)
             << ", time_slot=" << QString::fromStdString(time_slot);

    SaveRelationships();
}

void UserManage::updateTeacherTime(const std::string& teacher_name, const std::string& weekday,
                                   int start, int end) {
    auto& teacher = FindTeacher(teacher_name);
    teacher->remove_available_time(weekday, start, end);
    qDebug() << "Removed time slot for teacher" << QString::fromStdString(teacher_name)
             << ": " << QString::fromStdString(weekday) << "," << start << "-" << end;

    std::vector<std::string> lines;
    std::ifstream in(teacher_file);
    if (!in) {
        qDebug() << "Failed to open teacher_info.txt for reading";
        throw std::runtime_error("Failed to open teacher file");
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::vector<std::string> parts;
        std::stringstream ss(line);
        std::string part;
        while (std::getline(ss, part, '|')) {
            parts.push_back(part);
        }
        if (parts.size() >= 8 && parts[0] == teacher_name) {
            std::string new_times;
            for (const auto& t : teacher->available_times) {
                if (!new_times.empty()) new_times += "/";
                new_times += t.first + "," + std::to_string(t.second.first) + "," +
                             std::to_string(t.second.second);
            }
            parts[7] = new_times.empty() ? "" : new_times;
            line = parts[0];
            for (size_t i = 1; i < parts.size(); ++i) {
                line += "|" + parts[i];
            }
        }
        lines.push_back(line);
    }
    in.close();

    std::ofstream out(teacher_file);
    if (!out) {
        qDebug() << "Failed to open teacher_info.txt for writing";
        throw std::runtime_error("Failed to open teacher file for writing");
    }
    for (const auto& l : lines) {
        out << l << "\n";
    }
    out.close();

    qDebug() << "Updated teacher_info.txt for" << QString::fromStdString(teacher_name);
}

void UserManage::LoadRelationships(const std::string& file) {
    std::ifstream ifs(file);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open relationships: " << file << Qt::endl;
        return;
    }
    relationships.clear();
    std::string line;
    while (std::getline(ifs, line)) {
        std::istringstream iss(line);
        std::string student_name, teacher_name, subject, time_slot;
        if (std::getline(iss, student_name, '|') &&
            std::getline(iss, teacher_name, '|') &&
            std::getline(iss, subject, '|') &&
            std::getline(iss, time_slot)) {
            relationships.emplace_back(student_name, teacher_name, subject, time_slot);
        }
    }
    ifs.close();
}

void UserManage::SaveRelationships() {
    std::ofstream ofs(relationship_file);
    if (!ofs) {
        std::cerr << "Failed to open relationships file for writing: " << relationship_file << Qt::endl;
        throw std::runtime_error("Failed to open relationships.txt");
    }
    for (const auto& rel : relationships) {
        ofs << std::get<0>(rel) << "|" << std::get<1>(rel) << "|" << std::get<2>(rel) << "|"
            << std::get<3>(rel) << "\n";
    }
    qDebug() << "Saved " << relationships.size() << " relationships to " << QString::fromStdString(relationship_file);
    ofs.close();
}

void UserManage::AddRecord(const std::string& student_name, const std::string& teacher_name,
                           const std::string& subject, int score, const std::string& date) {
    if (score < 0 || score > 150) {
        qDebug() << "Invalid score: " << score << " must be between 0 and 150";
        throw std::runtime_error("Invalid score: must be between 0 and 150");
    }

    records.emplace_back(student_name, teacher_name, subject, score, date);
    qDebug() << "Added record: student=" << QString::fromStdString(student_name)
             << ", teacher=" << QString::fromStdString(teacher_name)
             << ", subject=" << QString::fromStdString(subject)
             << ", score=" << score
             << ", date=" << QString::fromStdString(date);
    SaveRecords();
}

void UserManage::LoadRecords(const std::string& file) {
    std::ifstream ifs(file);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open records: " << file << ", creating records file." << Qt::endl;
        std::ofstream ofs(file);
        ofs.close();
        return;
    }

    records.clear();
    std::string line;
    while (std::getline(ifs, line)) {
        std::istringstream iss(line);
        std::string student_name, teacher_name, subject, score_str, date;
        if (std::getline(iss, student_name, '|') &&
            std::getline(iss, teacher_name, '|') &&
            std::getline(iss, subject, '|') &&
            std::getline(iss, score_str, '|') &&
            std::getline(iss, date)) {
            try {
                int score = std::stoi(score_str);
                if (score < 0 || score > 150) {
                    std::cerr << "Invalid score in record: " << score << ", skipping line: " << line << Qt::endl;
                    continue;
                }
                records.emplace_back(std::move(student_name), std::move(teacher_name), std::move(subject), score,
                                     std::move(date));
            } catch (const std::exception& e) {
                std::cerr << "Invalid record format: " << line << Qt::endl;
            }
        }
    }
    ifs.close();
}

void UserManage::SaveRecords() {
    std::ofstream ofs(record_file);
    if (!ofs) {
        std::cerr << "Failed to open records file for writing: " << record_file << Qt::endl;
        return;
    }

    for (const auto& rec : records) {
        ofs << std::get<0>(rec) << "|" << std::get<1>(rec) << "|" << std::get<2>(rec) << "|"
            << std::get<3>(rec) << "|" << std::get<4>(rec) << "\n";
    }
    std::cout << "Saved " << records.size() << " records to " << record_file << Qt::endl;
    ofs.close();
}

bool UserManage::Delete(const std::string& name) {
    auto user_it = users.find(name);
    if (user_it == users.end()) {
        return false;
    }

    users.erase(user_it);

    auto teacher_it = teachers.find(name);
    if (teacher_it != teachers.end()) {
        teachers.erase(teacher_it);
    }

    relationships.erase(
        std::remove_if(relationships.begin(), relationships.end(),
            [&name](const auto& rel) {
                return std::get<0>(rel) == name || std::get<1>(rel) == name;
            }),
        relationships.end());

    records.erase(
        std::remove_if(records.begin(), records.end(),
            [&name](const auto& rec) {
                return std::get<0>(rec) == name || std::get<1>(rec) == name;
            }),
        records.end());

    SaveUsers();
    SaveTeachers();
    SaveRelationships();
    SaveRecords();

    std::cout << "Successfully deleted: " << name << Qt::endl;
    return true;
}