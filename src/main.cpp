/*
Some Bug
1.如果学生匹配这个老师后，是否需要修改这个老师的信息，也就是这个老师的时间已被占用，防止其他学生进行匹配已经占用时间的该老师。
2.导入成绩并选择分析成绩功能后，学生通过折线图分析近来历次考试成绩变化，老师可通过哪种类型图像来分析自己所教学生成绩的不同变化？我想导入成绩格式应该包含：
考试日期，科目，成绩
*/

/*
当学生匹配五个老师后，学生应该可以选择【1】【2】【3】【4】【5】哪位老师,从而绑定这两个师生关系，并且老师的可用时间会进行修改。但是需要改动teacher_info.txt中的内容吗？还是改动其他的地方？此外学生输入成绩和老师需要再创建一些txt文件吗？
*/
#include "user_manage.hpp"
#include "matching.hpp"
#include "analysis.hpp"
#include <iostream>
#include <string>
#include <set>
#include <sstream>
#include <SFML/Graphics.hpp>
#include <limits>

using namespace std;

const set<string> valid_districts = {
    "渝中区", "大渡口区", "江北区", "沙坪坝区", "九龙坡区",
    "南岸区", "北碚区", "渝北区", "巴南区"
};

const set<string> valid_weekdays = {
    "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

bool isValidTime(int time) {
    int hour = time / 100;
    int minute = time % 100;
    return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
}

string formatTimeSlot(const pair<string, pair<int, int>>& time_slot) {
    stringstream ss;
    ss << time_slot.first << "," << time_slot.second.first << "," << time_slot.second.second;
    return ss.str();
}

bool hasStudentTimeConflict(const vector<tuple<string, string, string, string>>& relationships,
                           const string& student_name, const string& day, int start, int end) {
    for (const auto& rel : relationships) {
        if (get<0>(rel) != student_name) continue;
        string time_slot = get<3>(rel);
        stringstream ss(time_slot);
        string rel_day, rel_start_str, rel_end_str;
        if (getline(ss, rel_day, ',') && getline(ss, rel_start_str, ',') && getline(ss, rel_end_str)) {
            try {
                int rel_start = stoi(rel_start_str);
                int rel_end = stoi(rel_end_str);
                if (rel_day == day && start < rel_end && end > rel_start) {
                    cout << "[Main] Time conflict detected for student " << student_name
                         << ": existing slot " << time_slot << " overlaps with requested "
                         << day << "," << start << "," << end << endl;
                    return true;
                }
            } catch (const exception& e) {
                cerr << "[Main] Invalid time slot format in relationship: " << time_slot << endl;
            }
        }
    }
    return false;
}

pair<int, int> adjustTimeSlot(const Teacher& teacher, const string& day, int start, int end) {
    for (const auto& time : teacher.available_times) {
        if (time.first != day) continue;
        int t_start = time.second.first;
        int t_end = time.second.second;
        if (start <= t_end && end >= t_start) {
            // Return the overlapping portion
            int new_start = max(start, t_start);
            int new_end = min(end, t_end);
            cout << "[Main] Adjusted time slot from " << day << "," << start << "," << end
                 << " to " << day << "," << new_start << "," << new_end << endl;
            return {new_start, new_end};
        }
    }
    cout << "[Main] No matching teacher time slot found for " << day << "," << start << "," << end << endl;
    return {-1, -1}; 
}

int main() {
    UserManage manage("/home/jqz/Desktop/Smart_CMS/users.txt", "/home/jqz/Desktop/Smart_CMS/teacher_info.txt");
    MatchingManager matchingManager(manage);

    string current_user;
    string current_type; 

    while (true) {
        cout << "选择功能: [1] 注册 [2] 登录 [3] 退出" << endl;
        int op;
        if (!(cin >> op)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "无效输入，请输入数字！" << endl;
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (op) {
        case 1: {
            cout << "请输入用户类型: [0] 学生 [1] 老师" << endl;
            string type;
            getline(cin, type);
            if (type != "0" && type != "1") {
                cout << "无效用户类型！" << endl;
                break;
            }
            cout << "请输入用户名和密码" << endl;
            string name, passwd;
            getline(cin, name);
            getline(cin, passwd);
            if (name.empty() || passwd.empty()) {
                cout << "用户名或密码不能为空！" << endl;
                break;
            }
            cout << "[Main] Registering user: type=" << type << ", name=" << name << endl;
            manage.Registered(type, name, passwd);
            break;
        }
        case 2: {
            cout << "请输入用户类型、用户名和密码" << endl;
            string type, name, passwd;
            getline(cin, type);
            getline(cin, name);
            getline(cin, passwd);
            if (type.empty() || name.empty() || passwd.empty()) {
                cout << "输入不能为空！" << endl;
                break;
            }
            cout << "[Main] Logging in: type=" << type << ", name=" << name << endl;
            if (manage.Login(type, name, passwd)) {
                current_user = name;
                current_type = type;
                if (type == "0") { // 学生
                    ScoreManager scoreManager(name);
                    scoreManager.loadFromFile("/home/jqz/Desktop/Smart_CMS/student_records.txt");
                    while (true) {
                        cout << "请选择功能: [0] 选择导师 [1] 分析结果 [2] 录入成绩 [3] 退出登录" << endl;
                        int student_op;
                        if (!(cin >> student_op)) {
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            cout << "无效输入，请输入数字！" << endl;
                            continue;
                        }
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        if (student_op == 3) {
                            cout << "[Main] Logging out student: " << current_user << endl;
                            break;
                        }

                        switch (student_op) {
                        case 0: {
                            // 输入匹配条件
                            MatchingCriteria criteria;
                            cout << "请输入期望老师的学历 (0: 大学生, 1: 在职教师, 2: 特级教师): ";
                            getline(cin, criteria.education);
                            if (criteria.education != "0" && criteria.education != "1" && criteria.education != "2") {
                                cout << "无效学历，请重新选择功能" << endl;
                                continue;
                            }
                            cout << "请输入期望老师的性格 (0: 温和型, 1: 严格型): ";
                            getline(cin, criteria.character);
                            if (criteria.character != "0" && criteria.character != "1") {
                                cout << "无效性格，请重新选择功能" << endl;
                                continue;
                            }
                            cout << "请输入期望学科 (如: 英语): ";
                            getline(cin, criteria.subject);
                            if (criteria.subject.empty()) {
                                cout << "学科不能为空，请重新选择功能" << endl;
                                continue;
                            }
                            cout << "请输入期望地区 (如: 渝中区): ";
                            getline(cin, criteria.location);
                            if (valid_districts.find(criteria.location) == valid_districts.end()) {
                                cout << "无效地区，仅支持重庆主城九区，请重新选择功能" << endl;
                                continue;
                            }
                            cout << "请输入最低可接受价格: ";
                            string price_min_str;
                            getline(cin, price_min_str);
                            try {
                                criteria.price_min = stoi(price_min_str);
                            } catch (const std::exception& e) {
                                cout << "无效价格，请重新选择功能" << endl;
                                continue;
                            }
                            cout << "请输入最高可接受价格: ";
                            string price_high_str;
                            getline(cin, price_high_str);
                            try {
                                criteria.price_high = stoi(price_high_str);
                            } catch (const std::exception& e) {
                                cout << "无效价格，请重新选择功能" << endl;
                                continue;
                            }
                            if (criteria.price_min > criteria.price_high) {
                                cout << "价格范围无效，请重新选择功能" << endl;
                                continue;
                            }
                            cout << "请输入期望时间段 (格式: 星期 开始时间 结束时间，如: Wed 1200 1400): ";
                            string day;
                            int start, end;
                            getline(cin, day);
                            stringstream ss(day);
                            if (!(ss >> day >> start >> end)) {
                                cout << "无效时间格式，请重新选择功能" << endl;
                                continue;
                            }
                            if (valid_weekdays.find(day) == valid_weekdays.end()) {
                                cout << "无效星期，请重新选择功能" << endl;
                                continue;
                            }
                            if (!isValidTime(start) || !isValidTime(end) || start >= end) {
                                cout << "无效时间格式或范围，请重新选择功能" << endl;
                                continue;
                            }
                            criteria.time_slot = {day, {start, end}};

                            // 检查学生时间冲突
                            if (hasStudentTimeConflict(manage.GetRelationships(), current_user, day, start, end)) {
                                cout << "时间段与已有绑定冲突，请选择其他时间" << endl;
                                continue;
                            }

                            // 进行匹配
                            cout << "[Main] Performing teacher matching for student: " << current_user << endl;
                            auto results = matchingManager.matchTeachers(criteria);
                            if (results.empty()) {
                                cout << "未找到符合条件的老师！" << endl;
                            } else {
                                cout << "匹配结果 (最多显示 5 位老师):" << endl;
                                for (size_t i = 0; i < results.size(); ++i) {
                                    cout << "[" << (i + 1) << "] 老师姓名: " << results[i].teacher->GetName()
                                         << ", 评分: " << results[i].score
                                         << ", 学历: " << results[i].teacher->education
                                         << ", 性格: " << results[i].teacher->character
                                         << ", 价格范围: [" << results[i].teacher->price_min
                                         << ", " << results[i].teacher->price_high << "]" << endl;
                                }

                                // 选择老师
                                cout << "请选择一位老师 (输入 1 到 " << results.size() << "): ";
                                int choice;
                                if (!(cin >> choice)) {
                                    cin.clear();
                                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                    cout << "无效输入，请输入数字！" << endl;
                                    continue;
                                }
                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                if (choice < 1 || choice > static_cast<int>(results.size())) {
                                    cout << "无效选择，请重新选择功能" << endl;
                                    continue;
                                }

                                // 绑定师生关系
                                auto& selected_teacher = results[choice - 1].teacher;
                                string teacher_name = selected_teacher->GetName();

                                // 调整时间段以匹配老师的可用时间
                                auto adjusted_time = adjustTimeSlot(*selected_teacher, day, start, end);
                                if (adjusted_time.first == -1) {
                                    cout << "老师的可用时间与请求时间不匹配，请选择其他老师或时间" << endl;
                                    continue;
                                }
                                pair<string, pair<int, int>> adjusted_slot = {day, adjusted_time};

                                // 再次检查调整后的时间是否与学生其他绑定冲突
                                if (hasStudentTimeConflict(manage.GetRelationships(), current_user, day,
                                                           adjusted_time.first, adjusted_time.second)) {
                                    cout << "调整后的时间段与已有绑定冲突，请选择其他时间" << endl;
                                    continue;
                                }

                                // 绑定关系并更新老师时间
                                cout << "[Main] Binding relationship: student=" << current_user
                                     << ", teacher=" << teacher_name << ", subject=" << criteria.subject
                                     << ", time_slot=" << formatTimeSlot(adjusted_slot) << endl;
                                manage.AddRelationship(current_user, teacher_name, criteria.subject,
                                                      formatTimeSlot(adjusted_slot));

                                // 更新老师可用时间
                                selected_teacher->remove_available_time(day, adjusted_time.first, adjusted_time.second);

                                // 保存更新后的老师信息和关系
                                manage.SaveTeachers();
                                manage.SaveRelationships();

                                cout << "已成功绑定与老师 " << teacher_name << " 的关系！" << endl;
                            }
                            break;
                        }
                        case 1: {
                            cout << "请输入要分析的学科: ";
                            string subject;
                            getline(cin, subject);
                            if (subject.empty()) {
                                cout << "学科不能为空，请重新选择功能" << endl;
                                continue;
                            }
                            // Validate subject against relationships
                            bool subject_exists = false;
                            for (const auto& rel : manage.GetRelationships()) {
                                if (std::get<0>(rel) == current_user && std::get<2>(rel) == subject) {
                                    subject_exists = true;
                                    break;
                                }
                            }
                            if (!subject_exists) {
                                cout << "未找到与该学科的绑定关系，请先选择导师" << endl;
                                continue;
                            }
                            cout << "[Main] Plotting scores for student: " << current_user << ", subject=" << subject << endl;
                            sf::RenderWindow window(sf::VideoMode(800, 600), "Student Score Analysis");
                            scoreManager.plotStudentScores(subject, window);
                            break;
                        }
                        case 2: {
                            // 录入成绩
                            cout << "请输入学科: ";
                            string subject;
                            getline(cin, subject);
                            if (subject.empty()) {
                                cout << "学科不能为空，请重新选择功能" << endl;
                                continue;
                            }
                            cout << "请输入成绩 (0-100): ";
                            string score_str;
                            int score;
                            getline(cin, score_str);
                            try {
                                score = stoi(score_str);
                            } catch (const std::exception& e) {
                                cout << "无效成绩，请重新选择功能" << endl;
                                continue;
                            }
                            if (score < 0 || score > 100) {
                                cout << "无效成绩，请重新选择功能" << endl;
                                continue;
                            }
                            // 使用当前日期（动态生成）
                            time_t now = time(nullptr);
                            char date[11];
                            strftime(date, sizeof(date), "%Y-%m-%d", localtime(&now));
                            // 查找学生的绑定关系以获取老师姓名
                            string teacher_name;
                            for (const auto& rel : manage.GetRelationships()) {
                                if (std::get<0>(rel) == current_user && std::get<2>(rel) == subject) {
                                    teacher_name = std::get<1>(rel);
                                    break;
                                }
                            }
                            if (teacher_name.empty()) {
                                cout << "未找到与该学科的绑定关系，请先选择导师" << endl;
                                continue;
                            }
                            cout << "[Main] Recording score: student=" << current_user << ", teacher=" << teacher_name
                                 << ", subject=" << subject << ", score=" << score << ", date=" << date << endl;
                            manage.AddRecord(current_user, teacher_name, subject, score, date);
                            manage.SaveRecords();
                            scoreManager.addScore(date, subject, static_cast<double>(score));
                            cout << "成绩录入成功！" << endl;
                            break;
                        }
                        default:
                            cout << "无效选项，请重新选择" << endl;
                        }
                    }
                } else if (type == "1") { // 老师
                    TeacherScoreAnalyzer analyzer(name);
                    analyzer.loadFromFile("/home/jqz/Desktop/Smart_CMS/student_records.txt");
                    while (true) {
                        cout << "请选择功能: [0] 修改个人信息 [1] 查看学生成绩 [2] 退出登录" << endl;
                        int teacher_op;
                        if (!(cin >> teacher_op)) {
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            cout << "无效输入，请输入数字！" << endl;
                            continue;
                        }
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        if (teacher_op == 2) {
                            cout << "[Main] Logging out teacher: " << current_user << endl;
                            break;
                        }
                        switch (teacher_op) {
                        case 0: {
                            cout << "修改个人信息功能尚未实现" << endl;
                            break;
                        }
                        case 1: {
                            cout << "请输入要分析的学科: ";
                            string subject;
                            getline(cin, subject);
                            if (subject.empty()) {
                                cout << "学科不能为空，请重新选择功能" << endl;
                                continue;
                            }
                            cout << "[Main] Plotting student scores for teacher: " << current_user << ", subject=" << subject << endl;
                            sf::RenderWindow window(sf::VideoMode(800, 600), "Teacher Score Analysis");
                            analyzer.plotTeacherAnalysis(subject, window);
                            break;
                        }
                        default:
                            cout << "无效选项，请重新选择" << endl;
                        }
                    }
                }
            } else {
                cout << "登录失败，请检查类型、用户名或密码" << endl;
            }
            break;
        }
        case 3: {
            cout << "退出程序" << endl;
            return 0;
        }
        default:
            cout << "无效选项，请重新选择" << endl;
        }
    }
}