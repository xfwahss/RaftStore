//
// Created by xfwahss on 2025/1/6.
//
#include <iomanip>
#include "Logger.h"

Logger *Logger::uniqueInstance = nullptr;

Logger *Logger::getInstance() {
    if (uniqueInstance == nullptr) {
        uniqueInstance = new Logger();
        uniqueInstance->logLevel = LogLevel::debug;
        uniqueInstance->target = LogTarget::terminal;
    }
    return uniqueInstance;
};

Logger::Logger() {
    this->dispTimeStamps = true;
    this->dispLogLevel = true;
    this->logLevel = LogLevel::debug;
    this->target = LogTarget::terminal;
    this->filePath = "log.txt";
}

void Logger::setLogLevel(LogLevel level) {
    this->logLevel = level;
}

void Logger::setLogTarget(Logger::LogTarget t) {
    this->target = t;
    if (target == LogTarget::file || target == LogTarget::file_and_terminal) {
        this->openLogFile();
    }
}

void Logger::setLogPath(std::string &path) {
    if (this->target == LogTarget::terminal) {
        this->setLogTarget(LogTarget::file);
    }
    this->filePath = path;
    this->openLogFile();
}

Logger::~Logger() {
    uniqueInstance->closeLogFile();
    delete uniqueInstance;
}

bool Logger::openLogFile() {
    if (outfile.is_open()) {
        outfile.close();
    }
    outfile.open(filePath, std::ios::app);
    return outfile.is_open();
}

void Logger::closeLogFile() {
    if (outfile.is_open()) {
        outfile.close();
    }
}

std::string Logger::currTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d %X");
    return ss.str();
}

std::string Logger::logLevelToString(Logger::LogLevel level) {
    switch (level) {
        case LogLevel::debug:
            return "[DEBUG]";
        case LogLevel::info:
            return "[INFO]";
        case LogLevel::warn:
            return "[WARN]";
        case LogLevel::error:
            return "[ERROR]";
        default:
            return "[UNKNOWN]";
    }
}


