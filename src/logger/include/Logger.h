//
// Created by xfwahss on 2025/1/6.
//

#ifndef LOGGER_H
#define LOGGER_H

#include<chrono>
#include<string>
#include<sstream>
#include<fstream>
#include<iostream>

class Logger {
public:
    enum class LogLevel {
        debug, info, warn, error
    };

    enum class LogTarget {
        file, terminal, file_and_terminal
    };

private:
    static Logger *uniqueInstance;
    std::ofstream outfile;
    std::string filePath;
    LogTarget target;
    LogLevel logLevel;
    bool dispTimeStamps;
    bool dispLogLevel;

    Logger();

    static std::string currTime();

    // convert LogLevel to string
    static std::string logLevelToString(LogLevel level);

    // basic log message helper
    void logMessageHelper(std::ostringstream &oss) {
        if (target == LogTarget::file) {
            if (outfile.is_open()) {
                outfile << oss.str() << std::endl;
            }
        } else if (target == LogTarget::terminal) {
            std::cout << oss.str() << std::endl;
        } else {
            std::cout << oss.str() << std::endl;
            if (outfile.is_open()) {
                outfile << oss.str() << std::endl;
            }
        }
    }

    // recursive log message helper
    template<typename T, typename... Args>
    void logMessageHelper(std::ostringstream &oss, T value, Args... args) {
        oss << value;
        logMessageHelper(oss, args...);
    }

    // main function to log message
    template<typename... Args>
    void logMessage(Logger::LogLevel level, Args... args) {
        if (level >= this->logLevel) {
            std::ostringstream oss;
            if (dispTimeStamps) {
                oss << "[" << currTime() << "]" << " ";
            }
            if (dispLogLevel) {
                oss << logLevelToString(level) << " ";
            }
            logMessageHelper(oss, args...);
        }
    }

    bool openLogFile();
    void closeLogFile();

public:


    static Logger* getInstance();

    void setLogLevel(LogLevel level);

    void setLogTarget(LogTarget target);

    void setLogPath(std::string &path);

    void setDispTimeStamps(bool disp);

    void setDispLogLevel(bool disp);


    ~Logger();

    template<typename... Arg>
    void info(Arg... args) {
        logMessage(LogLevel::info, args...);
    }

    template<typename... Arg>
    void debug(Arg... args) {
        logMessage(LogLevel::debug, args...);
    }

    template<typename... Arg>
    void warn(Arg... args) {
        logMessage(LogLevel::warn, args...);
    }

    template<typename... Arg>
    void error(Arg... args) {
        logMessage(LogLevel::error, args...);
    }
};

#endif //LOGGER_H
