#ifndef MATCHING_HPP
#define MATCHING_HPP
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "user_manage.hpp"

class PriceRBTree {
private:
    struct Node {
        uint16_t price;
        std::vector<Teacher*> teachers;
        Node *left, *right;
        bool isRed;
        Node(uint16_t p) : price(p), left(nullptr), right(nullptr), isRed(true) {}
    };
        
    Node* root;
        
    Node* insert(Node* h, uint16_t price, Teacher* teacher);
    Node* rotateLeft(Node* h);
    Node* rotateRight(Node* h);
    void flipColors(Node* h);
    bool isRed(Node* h) const { return h && h->isRed; }
    void queryRange(Node* h, uint16_t low, uint16_t high, std::vector<Teacher*>& result);

public:
    PriceRBTree() : root(nullptr) {}
    void insert(Teacher* teacher);
    std::vector<Teacher*> queryRange(uint16_t low, uint16_t high, const std::vector<Teacher*>& candidates);
    std::vector<Teacher*> intersect(const std::vector<Teacher*>& a, const std::vector<Teacher*>& b) const;
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
        Node *left, *right;
        Node(int start, int end) : interval(start, end), maxEnd(end), left(nullptr), right(nullptr) {}
    };
            
    Node* root;
            
    Node* insert(Node* h, Interval interval, Teacher* teacher);
    void queryOverlap(Node* h, int start, int end, std::vector<Teacher*>& result);

public:
    TimeIntervalTree() : root(nullptr) {}
    void insert(int start, int end, Teacher* teacher);
    std::vector<Teacher*> queryOverlap(const std::pair<int, int>& slot, const std::vector<Teacher*>& candidates);
    std::vector<Teacher*> intersect(const std::vector<Teacher*>& a, const std::vector<Teacher*>& b) const;
};

struct MatchingCriteria {
    std::string education; // "0" (大学生), "1" (在职教师), "2" (特级教师)
    std::string character; // "0" (温和型), "1" (严格型)
    std::string subject;   // 如 "英语"
    std::string location;  // 如 "渝中区"
    uint16_t price_min;
    uint16_t price_high;
    std::pair<std::string, std::pair<int, int>> time_slot; // 如 {"Wed", {1200, 1400}}
};

struct MatchResult {
    std::unique_ptr<Teacher> teacher;
    double score;
};

class MatchingManager {
public:
    MatchingManager(UserManage& userManage);
    std::vector<MatchResult> matchTeachers(const MatchingCriteria& criteria);

private:
    const UserManage& userManage;
    std::unordered_map<std::string, std::vector<Teacher*>> subject_index;
    std::unordered_map<std::string, std::vector<Teacher*>> location_index;
    PriceRBTree price_tree;
    TimeIntervalTree time_tree;

    double calculateScore(const Teacher& teacher, const MatchingCriteria& criteria);
    bool timeOverlap(const std::vector<std::pair<std::string, std::pair<int, int>>>& teacher_times,
                     const std::pair<std::string, std::pair<int, int>>& student_time);
    bool priceOverlap(uint16_t teacher_min, uint16_t teacher_max, uint16_t student_min, uint16_t student_max);
};

#endif