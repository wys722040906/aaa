#include <iostream>
#include <chrono>
#include <thread>
#include "simulator.h"
#include "utils.h"
#include "logger.h"

int main() {
    // 初始化日志系统
    Logger::init();
    Logger::log("系统启动");
    
    Simulator simulator;
    
    while (true) {
        std::cout << "\n=== 电梯模拟系统 ===" << std::endl;
        std::cout << "1. 开始模拟" << std::endl;
        std::cout << "2. 添加手动请求" << std::endl;
        std::cout << "3. 从文件加载请求" << std::endl;
        std::cout << "4. 重置系统" << std::endl;
        std::cout << "5. 显示统计" << std::endl;
        std::cout << "6. 退出" << std::endl;
        std::cout << "7. 系统配置" << std::endl;
        std::cout << "8. 调度策略管理" << std::endl;
        std::cout << "9. 显示性能报告" << std::endl;
        std::cout << "10. 显示能源报告" << std::endl;
        std::cout << "11. 维护管理" << std::endl;
        std::cout << "12. 数据分析" << std::endl;
        std::cout << "H. 帮助" << std::endl;
        
        char choice;
        std::cout << "\n请��择操作: ";
        std::cin >> choice;
        
        switch (choice) {
            case '1':
                simulator.start();
                while (simulator.isSimulationRunning()) {
                    simulator.update(0.1);
                    Utils::sleep(100);
                }
                break;
                
            case '2':
                simulator.handleManualRequest();
                break;
                
            case '3': {
                std::string filename;
                std::cout << "请输入数据文件名: ";
                std::cin >> filename;
                simulator.loadRequestsFromFile(filename);
                break;
            }
            
            case '4':
                simulator.reset();
                break;
                
            case '5':
                simulator.displayStatus();
                break;
                
            case '6':
                return 0;
                
            case '7':
                simulator.showConfigMenu();
                break;
                
            case '8':
                simulator.showDispatcherMenu();
                break;
                
            case '9':
                simulator.showPerformanceReport();
                break;
                
            case '0':
                simulator.showEnergyReport();
                break;
                
            case 'M':
                simulator.showMaintenanceMenu();
                break;
                
            case 'D':
                simulator.showDataAnalysisMenu();
                break;
                
            case 'H':
            case 'h': {
                std::string topic;
                std::cout << "请输入帮助主题（直接回车显示所有主题）: ";
                std::cin.ignore();
                std::getline(std::cin, topic);
                simulator.handleHelpCommand(topic);
                break;
            }
                
            default:
                std::cout << "无效选择" << std::endl;
        }
    }
    
    Logger::log("系统关闭");
    Logger::close();
    return 0;
} 