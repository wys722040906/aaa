#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>
#include <QChartView>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMap>

QT_CHARTS_USE_NAMESPACE

class StatisticsDialog : public QDialog {
    Q_OBJECT

public:
    explicit StatisticsDialog(QWidget *parent = nullptr);
    ~StatisticsDialog() = default;

    // 更新统计数据
    void updateStatistics(const QMap<int, int>& floorStatistics);
    // 清除统计数据
    void clearStatistics();

private:
    void setupUI();
    void setupChart();
    void updateChart();

    // UI组件
    QChartView* m_chartView;
    QPushButton* m_closeButton;
    
    // 图表组件
    QChart* m_chart;
    QBarSeries* m_series;
    QBarSet* m_barSet;
    QBarCategoryAxis* m_axisX;
    QValueAxis* m_axisY;
    
    // 数据
    QMap<int, int> m_statistics;  // floor -> usage count
};

#endif // STATISTICSDIALOG_H 