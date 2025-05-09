#include "user_manage.hpp"
#include "matching.hpp"
#include <iostream>
#include <string>
#include <set>

using namespace std;

// 重庆主城九区
const set<string> valid_districts = {
    "渝中区", "大渡口区", "江北区", "沙坪坝区", "九龙坡区",
    "南岸区", "北碚区", "渝北区", "巴南区"
};

// 有效星期
const set<string> valid_weekdays = {
    "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

// 验证时间格式 (1200 表示 12:00)
bool isValidTime(int time) {
    int hour = time / 100;
    int minute = time % 100;
    return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
}

int main() {
    UserManage manage("/home/jqz/Desktop/Smart_CMS/users.txt", "/home/jqz/Desktop/Smart_CMS/teacher_info.txt");
    MatchingManager matchingManager(manage);

    while (true) {
        cout << "选择功能: [1] 注册 [2] 登录 [3] 退出" << endl;
        int op;
        cin >> op;

        switch (op) {
        case 1: {
            cout << "请输入用户类型: [0] 学生 [1] 老师" << endl;
            string type;
            cin >> type;
            cout << "请输入用户名和密码" << endl;
            string name, passwd;
            cin >> name >> passwd;
            manage.Registered(type, name, passwd);
            break;
        }
        case 2: {
            cout << "请输入用户类型: [0] 学生 [1] 老师、用户名和密码" << endl;
            string type, name, passwd;
            cin >> type >> name >> passwd;
            if (manage.Login(type, name, passwd)) {
                cout << "登录成功！" << endl;
                if (type == "0") {
                    while (true) {
                        cout << "请选择功能: [0] 选择导师 [2] 录入成绩 [3] 分析结果 [4] 退出登录" << endl;
                        int student_op;
                        cin >> student_op;
                        if (student_op == 2) break;

                        switch (student_op) {
                        case 0: {
                            MatchingCriteria criteria;
                            cout << "请输入期望老师的学历 (0: 大学生, 1: 在职教师, 2: 特级教师): ";
                            cin >> criteria.education;
                            if (criteria.education != "0" && criteria.education != "1" && criteria.education != "2") {
                                cout << "无效学历，请重新选择功能" << endl;
                                continue;
                            }
                            cout << "请输入期望老师的性格 (0: 温和型, 1: 严格型): ";
                            cin >> criteria.character;
                            if (criteria.character != "0" && criteria.character != "1") {
                                cout << "无效性格，请重新选择功能" << endl;
                                continue;
                            }
                            cout << "请输入期望学科 (如: 英语): ";
                            cin >> criteria.subject;
                            cout << "请输入期望地区 (如: 渝中区): ";
                            cin >> criteria.location;
                            if (valid_districts.find(criteria.location) == valid_districts.end()) {
                                cout << "无效地区，仅支持重庆主城九区，请重新选择功能" << endl;
                                continue;
                            }
                            cout << "请输入您期望接受的最低价格: ";
                            cin >> criteria.price_min;
                            cout << "请输入您期望接受的最高价格: ";
                            cin >> criteria.price_high;
                            if (criteria.price_min > criteria.price_high) {
                                cout << "价格范围无效，请重新选择功能" << endl;
                                continue;
                            }
                            cout << "请输入期望时间段 (格式: 星期 开始时间 结束时间，如: Wed 1200 1400): ";
                            string day;
                            int start, end;
                            cin >> day >> start >> end;
                            if (valid_weekdays.find(day) == valid_weekdays.end()) {
                                cout << "无效星期，请重新选择功能" << endl;
                                continue;
                            }
                            if (!isValidTime(start) || !isValidTime(end) || start >= end) {
                                cout << "无效时间格式或范围，请重新选择功能" << endl;
                                continue;
                            }
                            criteria.time_slot = {day, {start, end}};

                            auto results = matchingManager.matchTeachers(criteria);
                            if (results.empty()) {
                                cout << "未找到符合条件的老师！" << endl;
                            } else {
                                cout << "匹配结果 (最多显示 5 位老师):" << endl;
                                for (const auto& result : results) {
                                    cout << "老师姓名: " << result.teacher->GetName()
                                         << ", 评分: " << result.score
                                         << ", 学历: " << result.teacher->education
                                         << ", 性格: " << result.teacher->character
                                         << ", 价格范围: [" << result.teacher->price_min
                                         << ", " << result.teacher->price_high << "]" << endl;
                                }
                            }
                            break;
                        }
                        case 1: {
                            cout << "分析结果功能尚未实现" << endl;
                            break;
                        }
                        default:
                            cout << "无效选项，请重新选择" << endl;
                        }
                    }
                } else if (type == "1") { 
                    cout << "请选择功能: [0] 修改个人信息" << endl;
                    int teacher_op;
                    cin >> teacher_op;
                    if (teacher_op == 0) {
                        cout << "修改个人信息功能尚未实现" << endl;
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