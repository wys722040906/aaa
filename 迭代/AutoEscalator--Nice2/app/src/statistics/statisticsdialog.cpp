#include "statisticsdialog.h"
#include <QApplication>
#include <QScreen>

StatisticsDialog::StatisticsDialog(QWidget *parent)
    : QDialog(parent)
    , m_chartView(new QChartView(this))
    , m_closeButton(new QPushButton("关闭", this))
    , m_chart(new QChart())
    , m_barSeries(new QBarSeries())
    , m_axisX(new QBarCategoryAxis())
    , m_axisY(new QValueAxis())
{
    setupUI();
    setupChart();
    setWindowTitle("楼层使用统计");
}

void StatisticsDialog::setupUI() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 设置图表视图
    m_chartView->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(m_chartView);

    // 添加关闭按钮
    mainLayout->addWidget(m_closeButton);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);

    // 设置窗口大小为屏幕的60%
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int width = screenGeometry.width() * 0.6;
    int height = screenGeometry.height() * 0.6;
    resize(width, height);
}

void StatisticsDialog::setupChart() {
    // 设置图表标题
    m_chart->setTitle("电梯系统楼层使用统计");
    m_chart->setAnimationOptions(QChart::SeriesAnimations);

    // 设置X轴（楼层）
    QStringList categories;
    for (int i = 1; i <= 14; ++i) {
        categories << QString::number(i) + "楼";
    }
    m_axisX->append(categories);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    // 设置Y轴（使用次数）
    m_axisY->setTitleText("使用次数");
    m_axisY->setLabelFormat("%d");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    // 设置柱状图系列
    QBarSet* initialSet = new QBarSet("乘客数量");
    for (int i = 0; i < 14; ++i) {
        initialSet->append(0);
    }
    m_barSeries->append(initialSet);
    
    m_chart->addSeries(m_barSeries);
    m_barSeries->attachAxis(m_axisX);
    m_barSeries->attachAxis(m_axisY);

    // 设置图表主题
    m_chart->setTheme(QChart::ChartThemeLight);
    
    // 设置图例可见
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);

    // 将图表添加到视图
    m_chartView->setChart(m_chart);
}

void StatisticsDialog::updateStatistics(const QMap<int, int>& stats) {
    // 更新楼层使用统计数据
    for (auto it = stats.begin(); it != stats.end(); ++it) {
        int floor = it.key();
        int count = it.value();
        m_floorStats[floor] = count;
    }
    
    // 更新图表
    updateChart();
}

void StatisticsDialog::updateChart() {
    // 清除现有数据
    m_barSeries->clear();
    
    // 创建数据集
    QBarSet* barSet = new QBarSet("乘客数量");
    
    // 添加每层楼的数据
    for (int floor = 1; floor <= 14; ++floor) {
        barSet->append(m_floorStats.value(floor, 0));
    }
    
    // 设置柱状图数据
    m_barSeries->append(barSet);
    
    // 更新坐标轴范围
    int maxValue = 0;
    for (int value : m_floorStats) {
        maxValue = qMax(maxValue, value);
    }
    m_axisY->setRange(0, maxValue + 10);  // 留出一些余量
    
    // 更新图表标题
    m_chart->setTitle(QString("楼层使用统计 (总乘客: %1人)").arg(calculateTotalPassengers()));
}

void StatisticsDialog::clearStatistics() {
    // 清空统计数据
    m_floorStats.clear();
    
    // 重置图表
    m_barSeries->clear();
    m_axisY->setRange(0, 100);  // 重置坐标轴范围
    m_chart->setTitle("楼层使用统计");
    
    // 添加空数据以保持图表结构
    QBarSet* emptySet = new QBarSet("乘客数量");
    for (int i = 0; i < 14; ++i) {
        emptySet->append(0);
    }
    m_barSeries->append(emptySet);
}

int StatisticsDialog::calculateTotalPassengers() const {
    int total = 0;
    for (int count : m_floorStats) {
        total += count;
    }
    return total;
} 