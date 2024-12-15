#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QtCharts>

QT_CHARTS_USE_NAMESPACE

class StatisticsDialog : public QDialog {
    Q_OBJECT

public:
    explicit StatisticsDialog(QWidget *parent = nullptr);
    
    // 更新统计数据
    void updateStatistics(const QMap<int, int>& stats);
    void clearStatistics();

private:
    void setupUI();
    void setupChart();
    void updateChart();
    int calculateTotalPassengers() const;  // 新增：计算总乘客数

private:
    // UI组件
    QChartView* m_chartView;
    QPushButton* m_closeButton;
    
    // 图表组件
    QChart* m_chart;
    QBarSeries* m_barSeries;
    QBarCategoryAxis* m_axisX;
    QValueAxis* m_axisY;
    
    // 数据存储
    QMap<int, int> m_floorStats;  // 楼层 -> 乘客数量
};

#endif // STATISTICSDIALOG_H 