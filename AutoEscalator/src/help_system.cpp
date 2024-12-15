#include "help_system.h"
#include <iostream>
#include <algorithm>

HelpSystem::HelpSystem() {
    initializeTopics();
}

void HelpSystem::initializeTopics() {
    topics["general"] = {
        "系统概述",
        "电梯模拟系统是一个模拟14层楼4部电梯运行的程序。\n"
        "系统支持多种调度策略，能源管理，维护管理等功能。",
        {"概述", "介绍", "帮助"}
    };
    
    topics["elevator"] = {
        "电梯操作",
        "每部电梯最多可载12人，运行速度为5秒/层。\n"
        "电梯空闲时最多等待10秒，然后返回1楼。",
        {"电梯", "运行", "载客"}
    };
    
    topics["dispatch"] = {
        "调度策略",
        "系统支持三种调度策略：\n"
        "1. 最近优先：选择最近的电梯响应请求\n"
        "2. 负载均衡：平衡各电梯的负载\n"
        "3. 节能模式：优化能源消耗",
        {"调度", "策略", "分配"}
    };
    
    topics["maintenance"] = {
        "维护管理",
        "系统提供完整的维护管理功能：\n"
        "1. 定期维护提醒\n"
        "2. 故障检测和报告\n"
        "3. 维护记录管理\n"
        "4. 故障修复跟踪",
        {"维护", "故障", "修理", "检修"}
    };
    
    topics["energy"] = {
        "能源管理",
        "能源管理系统包括：\n"
        "1. 实时能耗监控\n"
        "2. 能耗统计分析\n"
        "3. 节能建议\n"
        "4. 能耗报告生成",
        {"能源", "节能", "用电", "能耗"}
    };
    
    topics["statistics"] = {
        "数据统计",
        "系统提供多维度的数据统计：\n"
        "1. 运行时间统计\n"
        "2. 载客量统计\n"
        "3. 等待时间分析\n"
        "4. 使用频率分析",
        {"统计", "分析", "报告", "数据"}
    };
    
    topics["commands"] = {
        "操作命令",
        "主要操作命令：\n"
        "1. 开始模拟 - 启动系统模拟\n"
        "2. 手动请求 - 添加乘客请求\n"
        "3. 文件导入 - 从文件加载请求\n"
        "4. 系统配置 - 调整系统参数",
        {"命令", "操作", "使用", "控制"}
    };
    
    topics["config"] = {
        "系统配置",
        "可配置的系统参数：\n"
        "1. 电梯运行速度\n"
        "2. 最大载客量\n"
        "3. 等待时间限制\n"
        "4. 维护间隔时间",
        {"配置", "设置", "参数", "调整"}
    };
    
    // 添加更多主题...
}

void HelpSystem::showHelp(const std::string& topic) const {
    if (topic.empty()) {
        showTopicList();
        return;
    }
    
    auto it = topics.find(topic);
    if (it != topics.end()) {
        const auto& helpTopic = it->second;
        std::cout << "\n=== " << helpTopic.title << " ===" << std::endl;
        std::cout << helpTopic.content << std::endl;
    } else {
        std::cout << "未找到相关主题，请尝试以下关键词搜索：" << std::endl;
        auto results = searchHelp(topic);
        for (const auto& result : results) {
            std::cout << "- " << result << std::endl;
        }
    }
}

void HelpSystem::showTopicList() const {
    std::cout << "\n=== 帮助主题列表 ===" << std::endl;
    for (const auto& [key, topic] : topics) {
        std::cout << "- " << topic.title << " (使用 'help " << key << "'查看详情)"
                 << std::endl;
    }
}

std::vector<std::string> HelpSystem::searchHelp(const std::string& keyword) const {
    std::vector<std::string> results;
    
    for (const auto& [key, topic] : topics) {
        // 检查标题
        if (topic.title.find(keyword) != std::string::npos) {
            results.push_back(topic.title);
            continue;
        }
        
        // 检查关键词
        for (const auto& kw : topic.keywords) {
            if (kw.find(keyword) != std::string::npos) {
                results.push_back(topic.title);
                break;
            }
        }
    }
    
    return results;
} 