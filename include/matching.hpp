#ifndef MATCHING_HPP
#define MATCHING_HPP

#include "user_manage.hpp"
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <utility>

struct MatchingCriteria {
    std::string education; // 0: 大学生, 1: 在职教师, 2: 特级教师
    std::string character; // 0: 温和型, 1: 严格型
    std::string subject;
    std::string location;
    uint16_t price_min;
    uint16_t price_high;
    std::pair<std::string, std::pair<int, int>> time_slot;
};

struct MatchResult {
    std::unique_ptr<Teacher> teacher;
    double score;
};

class PriceRBTree {
private:
    struct Node {
        uint16_t price;
        std::vector<Teacher*> teachers;
        Node* left = nullptr;
        Node* right = nullptr;
        bool isRed = true;
        Node(uint16_t p) : price(p) {}
    };
    Node* root = nullptr;

    Node* rotateLeft(Node* h);
    Node* rotateRight(Node* h);
    void flipColors(Node* h);
    bool isRed(Node* h) const { return h && h->isRed; }
    Node* insert(Node* h, uint16_t price, Teacher* teacher);
    void queryRange(Node* h, uint16_t low, uint16_t high, std::vector<Teacher*>& result);

public:
    void insert(Teacher* teacher);
    std::vector<Teacher*> intersect(const std::vector<Teacher*>& a, const std::vector<Teacher*>& b) const;
    std::vector<Teacher*> queryRange(uint16_t low, uint16_t high, const std::vector<Teacher*>& candidates);
};

class TimeIntervalTree {
private:
    struct Interval {
        int start, end;
        std::vector<Teacher*> teachers;
        Interval(int s, int e) : start(s), end(e) {}
    };
    struct Node {
        Interval interval;
        int maxEnd;
        Node* left = nullptr;
        Node* right = nullptr;
        Node(int start, int end) : interval(start, end), maxEnd(end) {}
    };
    Node* root = nullptr;

    Node* insert(Node* h, Interval interval, Teacher* teacher);
    void queryOverlap(Node* h, int start, int end, std::vector<Teacher*>& result);

public:
    void insert(int start, int end, Teacher* teacher);
    std::vector<Teacher*> intersect(const std::vector<Teacher*>& a, const std::vector<Teacher*>& b) const;
    std::vector<Teacher*> queryOverlap(const std::pair<int, int>& slot, const std::vector<Teacher*>& candidates);
};

class MatchingManager {
private:
    UserManage& userManage;
    std::map<std::string, std::vector<Teacher*>> subject_index;
    std::map<std::string, std::vector<Teacher*>> location_index;
    PriceRBTree price_tree;
    TimeIntervalTree time_tree;

    double calculateScore(const Teacher& teacher, const MatchingCriteria& criteria);
    bool timeOverlap(const std::vector<std::pair<std::string, std::pair<int, int>>>& teacher_times,
                     const std::pair<std::string, std::pair<int, int>>& student_time);
    bool priceOverlap(uint16_t teacher_min, uint16_t teacher_max, uint16_t student_min, uint16_t student_max);

public:
    MatchingManager(UserManage& userManage);
    std::vector<MatchResult> matchTeachers(const MatchingCriteria& criteria);
};

#endif