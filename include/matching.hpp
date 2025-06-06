#ifndef MATCHING_HPP
#define MATCHING_HPP

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <utility>
#include "user_manage.hpp" // Ensure User and Teacher classes are defined here

struct Interval {
    int start, end;
    Interval(int s, int e) : start(s), end(e) {}
};

struct MatchingCriteria {
    std::string education;
    std::string character;
    std::string subject;
    std::string location;
    uint16_t price_min;
    uint16_t price_max;
    std::pair<std::string, std::pair<int, int>> time_slot;
};

struct MatchResult {
    Teacher* teacher;
    double score;
    std::pair<std::string, std::pair<int, int>> matched_time_slot;

    MatchResult(Teacher* t, double s, std::pair<std::string, std::pair<int, int>> mts)
        : teacher(t), score(s), matched_time_slot(std::move(mts)) {}
};

class PriceRBTree {
private:
    struct Node {
        uint16_t price;
        std::vector<Teacher*> teachers;
        Node* left;
        Node* right;
        Node* parent;
        bool isRed;
        Node(uint16_t p) : price(p), left(nullptr), right(nullptr), parent(nullptr), isRed(true) {}
        ~Node() {
            teachers.clear();
        }
    };
    Node* root;

    Node* rotateLeft(Node* h);
    Node* rotateRight(Node* h);
    void flipColors(Node* h);
    Node* insert(Node* h, uint16_t price, Teacher* teacher);
    void queryRange(Node* h, uint16_t min_price, uint16_t max_price, std::vector<Teacher*>& result);
    void destroyTree(Node* node);

public:
    PriceRBTree() : root(nullptr) {}
    ~PriceRBTree();
    void insert(Teacher* teacher);
    std::vector<Teacher*> queryRange(uint16_t min_price, uint16_t max_price);
    void reset();
};

class IntervalTree {
public:
    struct Interval {
        int start, end;
        Interval(int s, int e) : start(s), end(e) {}
    };

    IntervalTree() : root(nullptr) {}
    ~IntervalTree();
    void insert(int start, int end, const std::string& weekday, Teacher* teacher);
    std::vector<std::pair<Teacher*, std::pair<std::string, std::pair<int, int>>>>
        queryOverlap(const std::string& weekday, int start, int end);
    void reset();

private:
    struct Node {
        Interval interval;
        std::vector<Teacher*> teachers;
        std::string weekday;
        int maxEnd;
        std::unique_ptr<Node> left, right;
        Node(Interval i, const std::string& w) : interval(i), weekday(w), maxEnd(i.end), left(nullptr), right(nullptr) {}
        ~Node() {
            teachers.clear();
        }
    };
    std::unique_ptr<Node> root;

    void insert(std::unique_ptr<Node>& h, Interval interval, const std::string& weekday, Teacher* teacher);
    void queryOverlap(Node* h, const std::string& weekday, int start, int end,
                      std::vector<std::pair<Teacher*, std::pair<std::string, std::pair<int, int>>>>& result);
    void destroyTree(std::unique_ptr<Node>& node);
};

class MatchingManager {
public:
    MatchingManager(UserManage& userManage);
    void rebuildIndexes();
    std::vector<MatchResult> matchTeachers(const MatchingCriteria& criteria);

private:
    UserManage& userManage;
    std::map<std::string, std::vector<Teacher*>> subject_index;
    std::map<std::string, std::vector<Teacher*>> location_index;
    PriceRBTree price_tree;
    IntervalTree time_tree;
    double calculateScore(const Teacher& teacher, const MatchingCriteria& criteria);
    bool timeOverlap(const std::vector<std::pair<std::string, std::pair<int, int>>>& teacher_times,
                     const std::pair<std::string, std::pair<int, int>>& student_time);
    bool priceOverlap(uint16_t teacher_min, uint16_t teacher_max,
                      uint16_t student_min, uint16_t student_max);
};

#endif