#ifndef ANALYSIS_HPP
#define ANALYSIS_HPP

#include <string>
#include <vector>
#include <map>
#include <SFML/Graphics.hpp>

struct ScoreEntry {
    std::string date;    
    std::string subject; 
    double score;       
};

class ScoreManager {
private:
    std::vector<ScoreEntry> scores; 
    std::string user_id;          

public:
    ScoreManager(const std::string& id) : user_id(id) {}

    void addScore(const std::string& date, const std::string& subject, double score);

    std::vector<ScoreEntry> getScoresBySubject(const std::string& subject) const;

    void saveToFile(const std::string& filename) const;

    void loadFromFile(const std::string& filename);

    void plotStudentScores(const std::string& subject, sf::RenderWindow& window) const;
};

class TeacherScoreAnalyzer {
private:
    std::map<std::string, std::vector<ScoreEntry>> student_scores; 
    std::string teacher_id;                                    

public:
    TeacherScoreAnalyzer(const std::string& id) : teacher_id(id) {}

    void addStudentScores(const std::string& student_id, const std::vector<ScoreEntry>& scores);

    void plotTeacherAnalysis(const std::string& subject, sf::RenderWindow& window) const;

    void saveToFile(const std::string& filename) const;

    void loadFromFile(const std::string& filename);
};

#endif